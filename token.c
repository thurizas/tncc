#include "common.h"
#include "token.h"
#include "vector.h"
#include "node.h"

#include <stdio.h>
#include <stdlib.h>


void tok_print(void* data)
{
	if (NULL != data)
	{
		struct token* t = (struct token*)data;

		fprintf(stdout, "{ token type: %i, pos: (%i, %i), value:", t->type, t->pos.line, t->pos.col);
		if (t->type == TOKEN_TYPE_INT) fprintf(stdout, "%d", t->iVal);
		else if (t->type == TOKEN_TYPE_TYPE) fprintf(stdout, "%s", t->sVal);
		else if (t->type == TOKEN_TYPE_KEYWORD) fprintf(stdout, "%s", t->sVal);
		else if (t->type == TOKEN_TYPE_ID) fprintf(stdout, "%s", t->sVal);
		else if (t->type == TOKEN_TYPE_DECREMENT) fprintf(stdout, "%s", t->sVal);
		else fprintf(stdout, "%c", t->cVal);
		fprintf(stdout, "  }\n");
	}
}


char* getTokenName(struct token* tok)
{
	if ((tok->type == TOKEN_TYPE_ID) || (tok->type == TOKEN_TYPE_KEYWORD) || (tok->type == TOKEN_TYPE_TYPE) || (tok->type == TOKEN_TYPE_STRING))
		return tok->sVal;
	else
		return NULL;
}

static void tok_delete(struct token* token)
{
	//fprintf(stderr, "[+]   deleting token: "); tok_print((void*)token);
	if (NULL != token)
	{
		if ((token->type == TOKEN_TYPE_ID) || (token->type == TOKEN_TYPE_KEYWORD) ||
			(token->type == TOKEN_TYPE_TYPE) || (token->type == TOKEN_TYPE_STRING) ||
			(token->type == TOKEN_TYPE_BINOP))
		{
			free(token->sVal);
		}

		free(token);
	}
}


void tokens_clear(struct vec* tokens)
{
	fprintf(stderr, "[+] deleting tokens\n");

	if ((tokens->head != NULL) && (tokens->tail != NULL))
	{
		struct node* node = tokens->head;

		while (node != NULL)
		{
			struct node* nextNode = node->flink;
			tok_delete((struct token*)node->data);
			node = nextNode;
		}
	}
}


