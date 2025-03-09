#include "intrep.h"
#include "vector.h"
#include "node.h"
#include "astNode.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <memory.h>

static struct vec* ir_list = NULL;
static struct astNode* root = NULL;


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


// IR format: mnemonic,dest,op1,op2 or mnemonic,dest,op1
char* ir_genIR(struct astNode* astnode, bool* res)
{
    *res = false;
    bool cont = true;
    struct astNode* node = root;

    if (NULL != astnode) node = astnode;      // if called recursively, used passed in node

    // walk the ast tree and convert each node into TACKY
    while(cont)
    {
        switch (node->type)
        {
            case AST_TYPE_PROGRAM:
            {
                char* buf = tncc_calloc(11, sizeof(char));
                strncat(buf, "(PROGRAM)", 10);
                vec_enqueue(ir_list, 11, buf);
                struct node* vnode = root->prog.fncts->head;
                while (vnode != NULL)
                {
                    struct astNode* astnode = vnode->data;
                    char* buf = tncc_calloc(255, sizeof(char));
                    sprintf(buf, "(FUNCTION),%s,%s", astnode->fnct.name, astnode->fnct.retType);
                    vec_enqueue(ir_list, strlen(buf), buf);
                    ir_genIR(astnode, res);
                    vnode = vnode->flink;
                }
                cont = false;
            }
            break;

            case AST_TYPE_FUNCTION:
            {
                struct node* vnode = node->fnct.stmts->head;
                while (vnode != NULL)
                {
                    struct astNode* astnode = vnode->data;
                    ir_genIR(astnode, res);                        // process statement
                    vnode = vnode->flink;
                }

                cont = false;
            }
            break;

            case AST_TYPE_EXPR:
            {
                char* leftArg = NULL, *rightArg = NULL, *dstTok = NULL;
                
                if ((node->exp.left != NULL) && (node->exp.right == NULL))     // unitary operator (-, ~)
                {
                    dstTok = tempName();
                    leftArg = ir_genIR(node->exp.left, res);
                    char* buf = tncc_calloc(255, sizeof(char));
                    sprintf(buf, "%s,%s,%s", (strcmp(node->exp.op,"~") == 0 ? "COMP" : "NEG"), leftArg, dstTok);
                    vec_enqueue(ir_list, strlen(buf), buf);

                    //if (NULL != dstTok) { free(dstTok); dstTok = NULL; }
                    if (NULL != leftArg) { free(leftArg); leftArg = NULL; }
                }

                if (node->exp.right != NULL) ir_genIR(node->exp.right, res);
               
                return dstTok;                   // return where the value is stored.
                cont = false;
            }

            break;

            case AST_TYPE_STMT:
            {
                uint32_t type = node->stmt.type;

                switch(type)
                {
                    case AST_STMT_TYPE_RETURN:
                    {
                        struct astNode* retVal = node->stmt.returnStmt.exp;
                        char* dstTok = ir_genIR(retVal, res);
                        char* buf = tncc_calloc(255, sizeof(char));
                        sprintf(buf, "RET,%s", dstTok);
                        vec_enqueue(ir_list, strlen(buf), buf);

                        if (NULL != dstTok) { free(dstTok); dstTok = NULL; }

                        cont = false;
                    }
                    break;
                }
                break;
            }

            case AST_TYPE_INTVAL:
            {
                int val = node->iVal;
                char* dstTok = tempName();

                char* buf = tncc_calloc(255, sizeof(char));
                sprintf(buf, "MOV,%s,%d", dstTok, val);
                vec_enqueue(ir_list, strlen(buf), buf);

                return dstTok;                                      // return where the value is stored
            }
            break;
        }
    }

    *res = true;
    return NULL;
}


void ir_printIR(void* data)
{
    printf("%s\n", (char*)data);
}