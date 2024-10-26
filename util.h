#ifndef _util_h_
#define _util_h_

#include <stdint.h>
#include <stdbool.h>


void exitFailure(const char*, uint32_t);
bool isValidIdentifier(char);

// function pointer to print nodes in linked list.
void (*prt)(void*);

#endif
