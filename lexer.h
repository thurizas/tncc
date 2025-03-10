#ifndef _lexer_h_
#define _lexer_h_

#include "vector.h"

#include <stdbool.h>
#include <stdint.h>


bool lexer_init(const char*, uint8_t);
void lexer_deinit();

bool lexer_lex();

void lexer_print(const char*, const char*);

void tok_print(void*);  
struct vec* lexer_getTokens();


#endif
