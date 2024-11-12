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
        fprintf(stderr, "size of root of ast is: %zd\n", sizeof(struct astNode));
        memset(node, 0, sizeof(struct astNode));
        memcpy(node, _node, sizeof(struct astNode));

    }
    else
    {
        exitFailure("Memory allocation error creating astNode\n", -ERR_MEMORY);
    }

    return node;
}