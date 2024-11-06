/*
formal grammer:

program ::= function
function ::= "int" <identifier> "(" "void" ")" "{" <statements> "}"
<statement> ::= "return" <exp> ";"
<ext> ::= <int>
<identifier> = ? An identifire token ?
<int> ::= ? A constant token ?


parse_statement(tokens) :
  expect("return", tokens)
  return_val = parse_exp(tokens)
  expect(";", tokens)
  return return_val

expect(expected, tokens):
  actual = take_token(tokens)
  if actual != expected:
    fail("syntax error)

for example, the file test.c,
int main(void)
{
  return 2;
}

produces the following stream of tokens,
   { token type: 13, pos: (1, 2), value:int  }
 ->{ token type: 11, pos: (1, 6), value:main  }
 ->{ token type: 1, pos: (1, 10), value:(  }
 ->{ token type: 13, pos: (1, 11), value:void  }
 ->{ token type: 0, pos: (1, 15), value:)  }
 ->{ token type: 5, pos: (2, 2), value:{  }
 ->{ token type: 12, pos: (3, 2), value:return  }
 ->{ token type: 14, pos: (3, 9), value:2  }
 ->{ token type: 7, pos: (3, 10), value:;  }
 ->{ token type: 4, pos: (4, 2), value:}  }
 ->
*/

#include "common.h"
#include "token.h"
#include "parser.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <stdarg.h>

static struct vec* tokens;                     // vector of tokens we are working with
static uint32_t flags;                         // flags controlling operation

static void parse_program();
static bool parser_expect(int, const char*, struct vec*);

bool parser_init(struct vec* toks, uint8_t f) 
{ 
  bool res = true; 
  flags = f; 

  if (NULL != toks) 
  { 
	tokens = toks; 
  } 
  else 
  { 
	fprintf(stderr, "[-] Lexer produced no tokens, nothing to do.\n"); 
	res = false; 
  } 

  return res; 
} 

void parser_deinit()
{
    if (NULL != tokens)
        tokens = NULL;
}

bool parser_parse()
{
    bool res = false;

    vec_setCurrentNdx(tokens, 0);                                   // move to the first token

    if (parser_expect(TOKEN_TYPE_TYPE, "int", tokens))              // does the stream of tokens start with an int
    {
        vec_pop(tokens);                                            // move to next token
        parse_program();
    }

    return res;
}


static void parse_program()
{
    parser_expect(TOKEN_TYPE_ID, NULL, tokens);                  // check if we have an identifier
    // get name
    vec_pop(tokens);
    parser_expect(TOKEN_TYPE_LPAREN, NULL, tokens);
    vec_pop(tokens);
    parser_expect(TOKEN_TYPE_ID, "void", tokens);
    vec_pop(tokens);
    parser_expect(TOKEN_TYPE_RPAREN, NULL, tokens);
    vec_pop(tokens);
    parser_expect(TOKEN_TYPE_RCURLYB , NULL, tokens);
    vec_pop(tokens);
    parser_statement();
    parser_expect(TOKEN_TYPE_LCURLYB, NULL, tokens);
}

/*
   type -- the type of the token, one of the token type enumerations (see common.h)
   name -- name of the type (can be NULL if the expected type is a punctuation
   tokens -- vector of tokens
*/
static bool parser_expect(int type, const char* name, struct vec* tokens)
{
    bool res = false;

    struct token* t = vec_peekCurrent(tokens);            // peek at current token
    tok_print((void*)t);

    // check if token type is same a expected type, and if name if provided that token name matches
    if ((t->type == type) && (name != NULL ? strcmp(name, getTokenName(t)) == 0 : true))
    {
        res = true;
    }

    parser_error("[-] parse error, unexpected token at line %d, column %d", t->pos.line, t->pos.col);
    return res;
}

bool parser_statement()
{
    bool res = false;

    return res;
}

void parser_error(const char* fmt, ...)
{
  char  buf[1024];
  
  va_list args;
  va_start(args, fmt);

  vsprintf(buf, fmt, args);
  va_end(args);
  
  fprintf(stderr, "[-] parser error: %s\n", buf);
  exit(ERR_PARSE_FAILED);
}
