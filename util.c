#include "util.h"
#include "vector.h"
#include "astNode.h"
#include "common.h"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


// valid tokens for identifier are _ [a-zA-Z0-9]
bool isValidIdentifier(char ch)
{
    bool res = false;

    res = (ch == '_') || isalnum(ch);
    return res;
}

const char* types[] = { "int val", "string val", "char val", "real val", "statement", "function", "program" };
void printAST(struct vec* ast)
{
    if (NULL == ast)
    {
        printf("Abstract syntax tree not created yet\n");
    }
    else
    {
        if (ast->cntItems == 0) 
        {
            printf("Abstract syntax tree is empty\n");
            return;
        }

        // AST is a vector of astNodes's
        /*
            program node  +
                          |
                          +- function node (main)
                                |
                                + -- statement node
                                         |
                                         + return statement
                                                 |
                                                 +- expression statement
                                                             |
                                                             + -- value node (int)

        */
         
        vec_setCurrentNdx(ast, 0);
        struct astNode* node = vec_peekCurrent(ast);

        printf("node type is: %d[%s] \n", node->type, types[node->type]);

    }

}

void exitFailure(const char* msg, uint32_t code)
{
    fprintf(stderr, "[-] error: %d, %s\n", code, msg);
    exit(code);
}