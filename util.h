#ifndef _util_h_
#define _util_h_

#include <stdint.h>
#include <stdbool.h>

#include "vector.h"

#define SAFE_LIN(t) ((t) != NULL? (t)->pos.line: 0)
#define SAFE_COL(t) ((t) != NULL? (t)->pos.col:0)

// wrapper around calloc, checks for memory allocation failure
void* tncc_calloc(size_t num, size_t size);
char* tempName();
void exitFailure(const char*, uint32_t);
bool isValidIdentifier(char);

// function pointer to print nodes in linked list.
typedef void (*prt)(void*);

void printAST(struct vec*);

#endif
