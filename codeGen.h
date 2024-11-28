#ifndef _codeGen_h_
#define _codeGen_h_

#include "astNode.h"

#include <stdint.h>
#include <stdbool.h>

struct val
{
    uint32_t  type;
    union
    {
        int     iVal;
        float   fVal;
        double  dVal;
        char*   sVal;
    };
};



bool cg_init(struct astNode*, uint8_t);
void cg_deinit();

struct vec* cg_getAsm();

bool cg_genAsm();


void cg_printAsm(struct vec*);



#endif