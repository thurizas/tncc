#include "codeGen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "vector.h"
#include "buffer.h"
#include "node.h"
#include "astNode.h"

#define REG_CNT        8                            // number of general purpose registers available
#define PROLOG_LEN    48
#define MAX_FNCTS     64                            // maximum functions defined in one compilation unit 
#define ASM_INST_LEN 128                            // length of an assembly instruction

const char* prolog = "\tpush rbp\n\tmov rbp rsp\n"; // function prolog excluding stack manipulation

static struct vec* asmInsts = NULL;                 // vector to hold list of assembly instructions
static struct vec* irlist = NULL;                   // local copy of intermediate representation
static struct vec* symTab = NULL;                   // symbol table
static bool regs[REG_CNT] = { false };              // mapping of registers availabler8 through r15 are general purpose

struct tblEntry
{
    char* tmpVar;
    char* reg;
};

bool cg_init(struct vec* ir_list, uint8_t flags)
{
    bool res = false;

    if (NULL != ir_list)
    {
        irlist = ir_list;                        // get a local copy of the IR
        
        vec_init(&asmInsts);
        vec_init(&symTab);
    
        if ((asmInsts != NULL) && (symTab != NULL))
        {
            res = true;
        }
        else
        {
            if (asmInsts == NULL)
            {
                fprintf(stderr, "[-] failed to initialize assembly instruction list\n");
                exitFailure("failed to initialize assembly instrution list\n", -ERR_CODEGEN_FAILED);
            }

            if (symTab == NULL)
            {
                fprintf(stderr, "[-] failed to initialize symbol table \n");
                exitFailure("failed to initialize symbol table\n", -ERR_CODEGEN_FAILED);
            }
            
        }
    }
    else
    {
        fprintf(stderr, "[-] Intermediate Representation list is NULL.");
    }

    return res;
}

void cg_deinit()
{
    struct node* node = NULL;
    
    fprintf(stderr, "[+] freeing assembly instruction list\n");    
    node = asmInsts->head;
    while (node != NULL)
    {
        char* line = node->data;
        if (NULL != line) { free(line); line = NULL; }
        node = node->flink;
    }

    fprintf(stderr, "[+] freeing symbol table entries\n");
    node = symTab->head;
    while (node != NULL)
    {
        struct tblEntry* entry = node->data;
        if(NULL != entry)
        {
            if (NULL != entry->tmpVar) { free(entry->tmpVar); entry->tmpVar = NULL;}
            if (NULL != entry->tmpVar) { free(entry->reg); entry->reg = NULL;}
            free(entry);
        }

        node = node->flink;
    }

    vec_free(asmInsts);
    vec_free(symTab);
}



// PROGRAM - build list of exported functions .globl <fname>
// FUNCTION - add label '<fname>:'
//          - add prolog  push rbp; mov rbp rsp; sub rsp <stack size>
// RET      - move return value to rax
//            .section .note.GNU-stack,"",@progbits
/*
[? ] ir line is : (PROGRAM)
[? ] ir line is : (FUNCTION),main, int
[? ] ir line is : MOV, tmp.002, 2
[? ] ir line is : NEG, tmp.002, tmp.001
[? ] ir line is : COMP, tmp.001, tmp.000
[? ] ir line is : RET, tmp.000
*/

// extracts a function name for an IR FUNCTION ('(FUNCTION),main, int') line
char* getFnctName(char* line)
{ 
    char* loc;

    loc = strchr(line, ',');              // points to first comma
    char* end = strchr(loc + 1, ',');     // points to where name ends

    int64_t startNdx = loc - line + 1;    // starting index of name
    size_t len = end - loc - 1;           // length of name 

    char* fname = tncc_calloc(len+1 , sizeof(char));
    strncpy(fname, line + startNdx, len);
    fname[len] = '\0';

    return fname;
}

static void printSymTable()
{
    struct node* node = NULL;
    struct tblEntry* entry = NULL;

    node = symTab->head;
    fprintf(stderr, "symbol table is: ");

    if (symTab->cntItems == 0)
    {
        fprintf(stderr, "empty");
    }
    else
    {
        while (NULL != node)
        {
            entry = node->data;
            fprintf(stderr, "%s -> %s, ", entry->tmpVar, entry->reg);
            node = node->flink;
        }
    }
    fprintf(stderr, "\n");

}

// find first unused register and return its name 
static void genRegister(char** reg)
{
    uint32_t ndx;

    if (NULL != *reg) { free(*reg); *reg = NULL; }

    for (ndx = 0; ndx < REG_CNT; ndx++)
    {
        if (!regs[ndx]) { regs[ndx] = true;  break;}
    }

    if (ndx == REG_CNT)
    {
        fprintf(stderr, "[-] no register available for use for temporary variables\n");
        exitFailure("no register available for temporary vaiable", -ERR_CODEGEN_FAILED);
    }

    // 0->r8, 1->r9, 2->r10, ...., 7->r15
    *reg = tncc_calloc(5, sizeof(char));
    snprintf(*reg, 4, "r%d", (ndx+8));
}

static void resetRegisters()
{
    for (uint32_t ndx = 0; ndx < REG_CNT; ndx++)
    {
        regs[ndx] = false;
    }
}


static void updateMapping(char* tmpVar, char* reg)
{
    struct tblEntry* entry = tncc_calloc(1, sizeof(struct tblEntry));
    entry->tmpVar = tncc_calloc(strlen(tmpVar) + 1, sizeof(char));
    strncpy(entry->tmpVar, tmpVar, strlen(tmpVar));
    entry->reg = tncc_calloc(strlen(reg) + 1, sizeof(char));
    strncpy(entry->reg, reg, strlen(reg));
    vec_enqueue(symTab, sizeof(struct tblEntry), (void*)entry);
}

static char* lookUpTmpVar(char* tmpVar)
{
    char* reg = NULL;

    if (symTab != NULL)
    {
        if (!((symTab->head == NULL) && (symTab->tail == NULL)))
        {
            struct node* node = symTab->head;
            while (node != NULL)
            {
                struct tblEntry* entry = node->data;
                if (strcmp(entry->tmpVar, tmpVar) == 0)
                {
                    reg = entry->reg;
                    break;
                }
                node = node->flink;
            }
        }
        else
        {
            fprintf(stderr, "[-] symbol table is empty\n");
        }
    }
    else
    {
        fprintf(stderr, "[-] symbol table does not exist\n");
        exitFailure("no symbol table\n", -ERR_CODEGEN_FAILED);
    }

    return reg;
}



bool cg_genAsm()
{
    char** fnctName = tncc_calloc(MAX_FNCTS, sizeof(char*));
    uint32_t fnctCnt = 0;
    bool res = false;
  
    char* line = NULL;
    
    struct node* node = irlist->head;
    while((NULL != node) && (node->data != NULL))
    {
        line = node->data;
        if (strcmp(line, "(PROGRAM)") == 0)
        {
             
        }
        else if (strstr(line, "(FUNCTION)") != NULL)
        {
            resetRegisters();                           // reset register mappings.

            char* fname = getFnctName(line);
            fnctName[fnctCnt++] = fname;
            
            // generate function label...
            char* buf = tncc_calloc(ASM_INST_LEN, sizeof(char));
            snprintf(buf, ASM_INST_LEN-1, "\n\n%s:\n", fname);
            vec_enqueue(asmInsts, ASM_INST_LEN, buf);
            
            // add function prolog
            int localVarCnt = 0;
            char* bufProlog = tncc_calloc(ASM_INST_LEN, sizeof(char));
            snprintf(bufProlog, ASM_INST_LEN-1, "%s\n", prolog);
            if (localVarCnt > 0)
            {
                strncat(bufProlog, "\tsub rsp 0x0\n", 17);
                // TODO : add space requited to bufProlog
            }
            vec_enqueue(asmInsts, ASM_INST_LEN, bufProlog);

            // iterate through IR list till a new function line or empty
            node = node->flink;
            while ((node != NULL) && (node->data != NULL) && (strstr(node->data,"(FUNCTION)") == NULL))
            {
                char* line = (char*)node->data;
                char* tokens[4] = { 0 };
                uint32_t ndx = 0;
                uint32_t startNdx = 0;

                char* loc = strstr(line, ",");
                while (NULL != loc)            // TODO : tokenize IR line ... get opcode, dest loc, op1, op2
                {
                    uint32_t endNdx = loc - line;
                    line[endNdx] = '\0';
                    tokens[ndx++] = line;
                    line = &line[endNdx + 1];
                    loc = strstr(line, ",");
                }
                tokens[ndx] = line;

                //ndx = 0;
                //while (tokens[ndx] != NULL)
                //{
                //    fprintf(stderr, "    [?] token is %s\n", tokens[ndx++]);
                //}
                //ndx = 0;
                //fprintf(stderr, "avaliable registers are: ");
                //for (ndx = 0; ndx < REG_CNT; ndx++)
                //{
                //    fprintf(stderr, "%d, ", regs[ndx]);
                //}
                //fprintf(stderr, "\n");
                //printSymTable();

                if (strcmp(tokens[0], "MOV") == 0)        // mov <reg>,<reg> or mov <reg>,<mem> or mov <reg>,<imm>
                {                    
                    char* reg = NULL;
                    char* buf = tncc_calloc(ASM_INST_LEN, sizeof(char));

                    reg = lookUpTmpVar(tokens[1]);
                    if (NULL == reg)
                    {
                        genRegister(&reg);
                        updateMapping(tokens[1], reg);
                    }
                   
                    snprintf(buf, ASM_INST_LEN, "\t%10s\t %3s,%s\n", "mov", reg, tokens[2]);
                    vec_enqueue(asmInsts, strlen(buf), (void*)buf);

                    if (NULL != reg) { free(reg); reg = NULL; }
                }
                else if (strcmp(tokens[0], "NEG") == 0)   // neg <reg> or neg <mem>
                {
                    char* reg = NULL;
                    char* buf = tncc_calloc(ASM_INST_LEN, sizeof(char));

                    reg = lookUpTmpVar(tokens[1]);
                    if (NULL == reg)
                    {
                        genRegister(&reg);
                        updateMapping(tokens[1], reg);
                    }
                    updateMapping(tokens[2], reg);             // x86 uses a single register in inst
                    snprintf(buf, ASM_INST_LEN, "\t%10s\t %3s\n", "neg", reg);
                    vec_enqueue(asmInsts, strlen(buf), (void*)buf);
                }
                else if (strcmp(tokens[0], "COMP") == 0)  // not <reg> or not <mem>
                {
                    char* reg = NULL;
                    char* buf = tncc_calloc(ASM_INST_LEN, sizeof(char));

                    reg = lookUpTmpVar(tokens[1]);
                    if (NULL == reg)
                    {
                        genRegister(&reg);
                        updateMapping(tokens[1], reg);
                    }
                    updateMapping(tokens[2], reg);             // x86 uses a single register in inst
                    snprintf(buf, ASM_INST_LEN, "\t%10s\t %3s\n", "not", reg);
                    vec_enqueue(asmInsts, strlen(buf), (void*)buf);

                    if (NULL != reg) { free(reg); reg = NULL; }
                }
                else if (strcmp(tokens[0], "RET") == 0)   // mov rax, reg; ret 
                {
                    char* reg = NULL;
                    char* buf = tncc_calloc(ASM_INST_LEN, sizeof(char));

                    reg = lookUpTmpVar(tokens[1]);

                    snprintf(buf, ASM_INST_LEN, "\t%10s\t %3s, %s\n\t%10s\n", "mov", "rax", reg, "ret");
                    vec_enqueue(asmInsts, strlen(buf), (void*)buf);
                }
                else
                {
                    fprintf(stderr, "unknown opcode in IR, opcode is: %s\n", tokens[0]);
                    exitFailure("unknown opcode in code generation", -ERR_CODEGEN_FAILED);
                }

                node = node->flink;
            }
        }
        else
        {
            fprintf(stderr, "Unexpected line in IR: %s\n", line);
            exitFailure("error generating ASM", -ERR_CODEGEN_FAILED);
        }

        if(node != NULL) node = node->flink;
    }

    // add exported function names to beginning of asm list.
    for (int32_t ndx = fnctCnt-1; ndx >= 0; ndx--)
    {
        char* buf = tncc_calloc(ASM_INST_LEN, sizeof(char));  // .globl <
        snprintf(buf, ASM_INST_LEN - 1, ".globl %s", fnctName[ndx]);
        vec_push(asmInsts, ASM_INST_LEN, (void*)buf);
    }

    // free function name list...
    if(fnctName != NULL)
    {
        for (uint32_t ndx = 0; ndx < fnctCnt; ndx++)
        {
            if (fnctName[ndx] != NULL) 
            { 
                free(fnctName[ndx]); 
                fnctName[ndx] = NULL;  }
        }

        free(fnctName);
    }
    res = true;

    return res;
}

struct vec* cg_getAsm()
{
    return asmInsts;
}


void cg_printAsm(struct vec* asmListing)
{
    struct node* node = NULL;

    node = asmListing->head;

    while (node != NULL)
    {
        char* line = node->data;
        fprintf(stdout, "%s\n", line);

        node = node->flink;
    }
}