#ifndef _token_h_
#define _token_h_

#include "vector.h"


void tok_print(void*); 
void tokens_clear(struct vec*);
char* getTokenName(struct token*);

#endif
