#include "util.h"
#include "vector.h"
#include "astNode.h"
#include "common.h"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


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
         
        vec_setCurrentNdx(ast, 0);
        struct astNode* node = vec_getCurrent(ast);

        printf("node type is: %d[%s] \n", node->type, types[node->type]);

    }

}


void* tncc_calloc(size_t num, size_t size)
{
    void* alloc = NULL;

    alloc = calloc(num, size);
    if (NULL == alloc)
    {
        exitFailure("Failed to allocate memory\n", -ERR_MEMORY);
    }

    return alloc;
}

char* tempName()
{
    static uint32_t  cnt = 0;
    char* tmp = tncc_calloc(9, sizeof(char));
    snprintf(tmp, 8, "tmp.%03d", cnt);

    cnt++;

    return tmp;
}

void exitFailure(const char* msg, uint32_t code)
{
    fprintf(stderr, "[-] error: %d, %s\n", code, msg);
    exit(code);
}