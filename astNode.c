#include "node.h"
#include "astNode.h"
#include "common.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


struct astNode* astNode_create(struct astNode* _node)
{
    struct astNode* node = malloc(sizeof(struct astNode));         // LEAKS 56 bytes, leak rcds 1, 2
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

void astNode_delExpr(struct astNode* _node)
{
    struct exp e = _node->exp;
    if(NULL != e.left) astNode_delete(e.left);
    if(NULL != e.right) astNode_delete(e.right);

    free(_node);
}

void astNode_delStmt(struct astNode* _node)
{
    struct stmt s = _node->stmt;
    switch (s.type)
    {
        case AST_STMT_TYPE_RETURN: astNode_delete(s.returnStmt.exp); break;
    }

    free(_node);
}

void astNode_delFnct(struct astNode* _node)
{
    struct fnct f = _node->fnct;
    if (NULL != f.retType){ free(f.retType); f.retType = NULL; }
    if (NULL != f.name){ free(f.name); f.name = NULL; }

    // iterate over vector of arguments and free individual data
    struct node* item = f.args->head;
    while (item != NULL)
    {
        free(item->data);
        item->data = NULL;
        item = item->flink;
    } 
    vec_free(f.args);

    // iterate over vector of statements and free individual statements
    //item = f.stmts->head;
    //while (NULL != item)
    //{
    //    astNode_delStmt(item->data);
    //    item = item->flink;
    //}
    vec_free(f.stmts);

    free(_node);
}

void astNode_delProg(struct prog prog)
{
    // iterate over function pointer in program
    struct node* item = prog.fncts->head;
    while (NULL != item)
    {
        astNode_delFnct(item->data);
        item = item->flink;
    }

    vec_free(prog.fncts);
}


void astNode_delete(struct astNode* _node)
{
    fprintf(stdout, "[+] deleteing the AST structure");
    switch (_node->type)
    {
    case AST_TYPE_EXPR: astNode_delExpr(_node); break;
    case AST_TYPE_STMT: astNode_delStmt(_node); break;
    case AST_TYPE_FUNCTION: astNode_delFnct(_node); break;
    case AST_TYPE_PROGRAM: astNode_delProg(_node->prog); break;
    default: printf("***Unknown AST type: %d\n", _node->type);
    }

    free(_node);
}



static const char* astNodeTypes[] = { "program node           ", "function node          ", "statement node         ", 
                                      "expression node        ", "integer constant       ", "string constant        ",
                                      "character constant     ", "floating point constant", "unitary opertor        "};
                                                                                            
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

        case AST_TYPE_UNOP:
            printf("%*soperator: %c\n", depth + 4, "", astnode->cVal);
            printf("%*sexpresion: ", depth + 4, "");
            astNode_print(astnode->exp.left, depth + 4);
            break;

        default:
            printf("***Unknown AST type: %d\n", astnode->type);

    }
    printf("%*s}\n", depth, "");
}