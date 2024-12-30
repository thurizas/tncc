#ifndef _intrep_h_
#define _intrep_h_

#include "astNode.h"

bool ir_init(struct astNode*, uint8_t);
void ir_deinit();

bool ir_genIR();




#endif