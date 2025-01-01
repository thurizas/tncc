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
        vec_free(ir_list);
        //free(ir_list);
    }
}

struct vec* ir_getIR()
{
    return ir_list;
}

/*
    type: 0 (program node           )
    nodes:
    {
        {
            type: 1 (function node          )
            name       : main
            return type: int
            arguments  : void,
            body       :
            {
                {
                    type: 2 (statement node         )
                    command    : return
                    expression :
                    {
                        {
                            type: 8 (unitary opertor        )
                            operator: ~
                            expresion:                             {
                                type: 8 (unitary opertor        )
                                operator: -
                                expresion:                                 {
                                    type: 4 (integer constant       )
                                    value: 2
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


*/
//bool ir_genIR(struct astNode* astnode, char* tempTok)
char* ir_genIR(struct astNode* astnode, bool* res)
{
    *res = false;
    bool cont = true;
    struct astNode* node = root;

    if (NULL != astnode) node = astnode;      // if called recursively, used passed in node

    // TODO : walk the ast tree and convert each node into TACKY

    while(cont)
    {
        switch (node->type)
        {
            case AST_TYPE_PROGRAM:
            {
                char* buf = tncc_calloc(11, sizeof(char));
                strncat(buf, "(PROGRAM)", 10);
                vec_push(ir_list, 11, buf);
                struct node* vnode = root->prog.fncts->head;
                while (vnode != NULL)
                {
                    struct astNode* astnode = vnode->data;
                    char* buf = tncc_calloc(255, sizeof(char));
                    sprintf(buf, "(FUNCTION), (NAME)%s, (RETVAL)%s", astnode->fnct.name, astnode->fnct.retType);
                    vec_push(ir_list, strlen(buf), buf);
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
                struct vec* instStk = NULL;
                char* leftArg = NULL, *rightArg = NULL, *dstTok = NULL;

                vec_init(&instStk);
                fprintf(stderr, "[?] got a expression \n");

                if ((node->exp.left != NULL) && (node->exp.right == NULL))     // unitary operator (-, ~)
                {
                    leftArg = ir_genIR(node->exp.left, res);
                    dstTok = tempName();
                    char* buf = tncc_calloc(255, sizeof(char));
                    sprintf(buf, "%s, %s, %s", node->exp.op, dstTok, leftArg);
                    vec_push(ir_list, strlen(buf), buf);
                }

                if (node->exp.right != NULL) ir_genIR(node->exp.right, res);

     
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
                        sprintf(buf, "RET, %s", dstTok);
                        vec_push(ir_list, strlen(buf), buf);

                        cont = false;
                    }
                    break;
                }
                break;
            }

            case AST_TYPE_INTVAL:
            {
                fprintf(stdout, "[?] got an interal literal\n");
                int val = node->iVal;
                char* tempTok = tempName();

                char* buf = tncc_calloc(255, sizeof(char));
                sprintf(buf, "MOV, %d, %s", val, tempTok);
                vec_push(ir_list, strlen(buf), buf);

                return tempTok;
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