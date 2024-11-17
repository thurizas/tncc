#include "node.h"
#include "astNode.h"
#include "common.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


struct astNode* astNode_create(struct astNode* _node)
{
    struct astNode* node = malloc(sizeof(struct astNode));
    if (NULL != node)
    {  
        memset(node, 0, sizeof(struct astNode));
        memcpy(node, _node, sizeof(struct astNode));
    }
    else
    {
        exitFailure("Memory allocation error creating astNode\n", -ERR_MEMORY);
    }

    return node;
}



static const char* astNodeTypes[] = { "program node    ", "function node   ", "statement node  ", 
                                      "expression node ", "integer constant"};

static void astNode_printExpt(struct exp* node, uint32_t depth)
{
    if (NULL == node->right)                       // only single argument
    {
        astNode_print(node->left, depth + 4);
    }
    else
    {
        astNode_print(node->left, depth + 4);
        printf("%*s%s", depth, "", node->op);
        astNode_print(node->right, depth + 4);
    }

}


static void astNode_printStmt(struct stmt* node, uint32_t depth)
{
    switch (node->type)
    {
        case AST_STMT_TYPE_RETURN:
            printf("%*scommand    : %s\n", depth, "", "return");
            printf("%*sexpression :\n", depth, "");
            printf("%*s{\n", depth, "");
            struct returnStmt  retStmt = node->returnStmt;
            astNode_print(retStmt.exp, depth + 4);
            printf("%*s}\n", depth, "");
            break;
    }
}


static void astNode_printFnct(struct fnct* node, uint32_t depth)
{
    printf("%*sname       : %s\n", depth, "", node->name);
    printf("%*sreturn type: %s\n", depth, "", node->retType);
    printf("%*sarguments  : ", depth, "");
    struct node* arg = node->args->head;
    while (NULL != arg)
    {
        printf("%s, ", (char*)arg->data);
        arg = arg->flink;
    }
    printf("\n");
    printf("%*sbody       :\n", depth, "");
    struct node* stmt = node->stmts->head;
    while (NULL != stmt)
    {
        printf("%*s{\n", depth, "");
        astNode_print(stmt->data, depth + 4);
        printf("%*s}\n", depth, "");
        stmt = stmt->flink;
    }
}


static void astNode_printPrgm(struct prog* prgmNode, uint32_t depth)
{
    struct node* pnode = prgmNode->fncts->head;

    while (NULL != pnode)
    {
        printf("%*s{\n", depth, "");
        astNode_print(pnode->data, depth+4);
        printf("%*s}\n", depth, "");

        pnode = pnode->flink;
    }
}

void astNode_print(struct astNode* astnode, uint32_t depth)
{
    printf("%*s{\n",depth, "");
    printf("%*s    type: %d (%s)\n", depth, "", astnode->type, astNodeTypes[astnode->type]);
    switch(astnode->type)
    {
        case AST_TYPE_EXPR:
            astNode_printExpt(&(astnode->exp), depth + 4);
            break;

        case AST_TYPE_STMT:
            astNode_printStmt(&(astnode->stmt), depth + 4);
            break;

        case AST_TYPE_FUNCTION:
            astNode_printFnct(&(astnode->fnct), depth + 4);
            break;

        case AST_TYPE_PROGRAM:
            printf("%*s    nodes:\n", depth, "");
            astNode_printPrgm(&(astnode->prog), depth + 4);
            break;

        case AST_TYPE_INTVAL:
            printf("%*svalue: %d\n", depth+4, "", astnode->iVal);
            break;

        default:
            printf("***Unknown AST type: %d\n", astnode->type);

    }
    printf("%*s}\n", depth, "");
}