#ifndef _parser_h_
#define _parser_h_

#include "common.h"

#include <stdbool.h>

//bool parser_init(struct vec*, uint8_t);
void parser_deinit();
bool parser_parse();
bool parser_statement();

void parser_error(const char* fmt, ...);

#endif
