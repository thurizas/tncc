#include "common.h"
#include "intrep.h"
#include "vector.h"
#include "node.h"
#include "astNode.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
//#include <memory.h>

static struct vec* ir_list = NULL;
static struct astNode* root = NULL;

void ir_errorAndExit(char*, ...);


bool ir_init(struct astNode* _root, uint8_t flags)
{
    bool res = false;

    vec_init(&ir_list);
    if (NULL != ir_list)
    {
        root = _root;
        res = true;
    }
    else
    {
        exitFailure("failed to allocate memory for IR instruction list", -ERR_MEMORY);
    }

    return res;
}


void ir_deinit()
{
    if (NULL != ir_list)
    {
        fprintf(stderr, "[+] freeing intermediate representation list\n");
        struct node* node = ir_list->head;
        while (NULL != node)
        {
            free(node->data);
            node->data = NULL;
            node = node->flink;
        }
        vec_free(ir_list);
    }
}

struct vec* ir_getIR()
{
    return ir_list;
}

void RLPostorder(struct astNode* root, struct vec* irlist)
{
    if (root != NULL)
    {
        int foo = root->type;
        switch (root->type)
        {
            case AST_TYPE_PROGRAM:
            {
                struct irnode* temp = tncc_calloc(1, sizeof(struct irnode));
                temp->name = tncc_calloc(11, sizeof(char));
                temp->type = IR_TYPE_PROGRAM;
                strncpy(temp->name, "PROGRAM", 10);
                vec_enqueue(irlist, sizeof(struct irnode), (void*)temp);

                //iterate over vector of functions
                if (root->prog.fncts != NULL)
                {                        
                    struct node* vnode = root->prog.fncts->head;
                    while (vnode != NULL)
                    {
                        struct astNode* fnctNode = vnode->data;

                        struct irnode* temp = tncc_calloc(1, sizeof(struct irnode));
                        temp->type = IR_TYPE_FUNCTION;
                        temp->name = tncc_calloc(266, sizeof(char));
                        snprintf(temp->name, 265, "(FUNCTION)%s,%s", fnctNode->fnct.retType, fnctNode->fnct.name);
                        vec_enqueue(irlist, sizeof(struct irnode), (void*)temp);
                        RLPostorder(fnctNode, irlist);
                        vnode = vnode->flink;
                    }
                }
                else
                {
                    ir_errorAndExit("No functions found in the AST");
                }
            }
            break;

            case AST_TYPE_FUNCTION:
            {
                struct node* vnode = root->fnct.stmts->head;

                //iterator over statements in function body...
                while (vnode != NULL)
                {
                    struct astNode* stmtnode = vnode->data;
                    //struct irnode* temp = tncc_calloc(1, sizeof(struct irnode));
                    //temp->type = IR_TYPE_STMT;
                    
                    //switch (stmtnode->stmt.type)        // build contents of statement node
                    //{
                        //case AST_STMT_TYPE_RETURN:
                            RLPostorder(stmtnode, irlist);
                           //break;
                    //}

                    vnode = vnode->flink;
                }
            }
            break;

            case AST_TYPE_STMT:
            {
                struct irnode* temp = tncc_calloc(1, sizeof(struct irnode));
                temp->type = IR_TYPE_STMT;
                temp->name = tncc_calloc(255, sizeof(char));
                snprintf(temp->name, 10, "STATEMENT:");

                switch (root->stmt.type)
                {
                    case AST_STMT_TYPE_RETURN:
                    {
                        strncat(temp->name, " return", 8);
                        RLPostorder(root->stmt.returnStmt.exp,irlist);
                        break;
                    }

                    default:
                        ir_errorAndExit("Unsupported AST node type, %d\n", root->stmt.type);
                }

                vec_enqueue(irlist, sizeof(struct irnode), temp);
            }
            break;

            case AST_TYPE_EXPR:
            {
                struct irnode* temp = tncc_calloc(1, sizeof(struct irnode));
                temp->type = IR_TYPE_EXPR;
                temp->name = tncc_calloc(255, sizeof(char));
                strncpy(temp->name, "EXPRESSION:", 12);
                temp->op = tncc_calloc(4, sizeof(char));
                strncpy(temp->op, root->exp.op, 3);

                RLPostorder(root->exp.right, irlist);
                RLPostorder(root->exp.left, irlist);

                vec_enqueue(irlist, sizeof(struct irnode), temp);
            }
            break;

            case AST_TYPE_INTVAL:
            {
                struct irnode* temp = tncc_calloc(1, sizeof(struct irnode));
                temp->type = IR_TYPE_INT_LITERAL;
                temp->name = tncc_calloc(12, sizeof(char));
                snprintf(temp->name, 11, "%d", root->iVal);
                temp->iVal = root->iVal;

                vec_enqueue(irlist, sizeof(struct irnode), temp);
            }
            break;

            default:
                ir_errorAndExit("Unknown AST node type %d\n", root->type);
        }
    }
}

// IR format: mnemonic,dest,op1,op2 or mnemonic,dest,op1
char* ir_genIR(struct astNode* astnode, bool* res)
{
    *res = false;
    bool cont = true;
    struct astNode* node = root;

    RLPostorder(root, ir_list);                 // flatten the AST into a list :right-to-left post-order traversal

    // TODO: take flattened AST and use to emit IR
    *res = true;
    return NULL;
}

void ir_errorAndExit(char* fmt, ...)
{
  char  buf[1024];

  va_list args;
  va_start(args, fmt);

  vsprintf(buf, fmt, args);
  va_end(args);

  fprintf(stderr, "[-] IR generator error: %s\n", buf);
  exit(ERR_INTREP_FAILED);    
}

void ir_printIR(void* data)
{
    struct irnode* tmp = (struct irnode*)data;
    printf("%3s, %i, %s\n", (tmp->op == NULL ? "": tmp->op), tmp->type, tmp->name);
}

