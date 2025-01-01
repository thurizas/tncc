#ifndef _intrep_h_
#define _intrep_h_

#include "astNode.h"

bool ir_init(struct astNode*, uint8_t);
void ir_deinit();

struct vec* ir_getIR();

//bool ir_genIR(struct astNode*, char**);
char* ir_genIR(struct astNode*, bool*);

void ir_printIR(void*);




#endif