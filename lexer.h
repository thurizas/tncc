#ifndef _lexer_h_
#define _lexer_h_

#include "vector.h"

#include <stdbool.h>
#include <stdint.h>


bool lexer_init(const char*, uint8_t);
void lexer_deinit();

void lexer_lex();



#endif
