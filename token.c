#include "common.h"
#include "token.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>


void tok_print(void* data)
{
	struct token* t = (struct token*)data;

	fprintf(stdout, "{ token type: %i, pos: (%i, %i), value:", t->type, t->pos.line, t->pos.col);
	if (t->type == TOKEN_TYPE_INT) fprintf(stdout, "%d", t->iVal);
	else if (t->type == TOKEN_TYPE_TYPE) fprintf(stdout, "%s", t->sVal);
	else if (t->type == TOKEN_TYPE_KEYWORD) fprintf(stdout, "%s", t->sVal);
	else if (t->type == TOKEN_TYPE_ID) fprintf(stdout, "%s", t->sVal);
	else fprintf(stdout, "%c", t->cVal);
	fprintf(stdout, "  }\n");
}


char* getTokenName(struct token* tok)
{
	if ((tok->type == TOKEN_TYPE_ID) || (tok->type == TOKEN_TYPE_KEYWORD) || (tok->type == TOKEN_TYPE_TYPE) || (tok->type == TOKEN_TYPE_STRING))
		return tok->sVal;
	else
		return NULL;
}

