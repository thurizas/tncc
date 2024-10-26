#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


// valid tokens for identifier are _ [a-zA-Z0-9]
bool isValidIdentifier(char ch)
{
    bool res = false;

    res = (ch == '_') || isalnum(ch);
    return res;
}

void exitFailure(const char* msg, uint32_t code)
{
    fprintf(stderr, "[-] error: %d, %s\n", code, msg);
    exit(code);
}