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
#include "util.h"
#include "token.h"
#include "parser.h"
#include "astNode.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <stdarg.h>


static struct vec* tokens;                     // vector of tokens we are working with
static uint32_t flags;                         // flags controlling operation


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

//<ext> ::= <int>
static struct astNode* parse_expression(struct vec* ast)
{
    struct astNode* node = NULL;

    struct token* token = vec_peekCurrent(tokens);

    if ((token != NULL) && (token->type == TOKEN_TYPE_INT))
    {
        node = astNode_create(&(struct astNode){.type = AST_TYPE_INTVAL, .iVal = token->iVal});
        if (NULL != ast)
        {
            vec_push(ast, node);
        }

        vec_pop(tokens);                                    // remove the numeric literal node
        return node;
    }
    else
    {
        parserErrorAndExit("Unknown or unexpected token at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
    }

    return node;
}


//<statement> :: = "return" < exp > ";"
static struct astNode* parse_statement(struct vec* ast)
{
    // eat optional currely brace if present
    if (((struct token*)vec_peekCurrent(tokens))->type == TOKEN_TYPE_LCURLYB)
        vec_pop(tokens);

    struct token* token = vec_peekCurrent(tokens);                      // get the command

    if ((token != NULL) && (token->type = TOKEN_TYPE_KEYWORD) && (strcmp(token->sVal, "return") == 0))
    {
        vec_pop(tokens);                                               // eat 'return'
        struct astNode* expNode = parse_expression(NULL);
        if (!parser_expect(TOKEN_TYPE_SEMICOLON, NULL, tokens))
        {
            struct token* t = vec_peekCurrent(tokens);
            parserErrorAndExit("expected semicolon at line: %d, column %d\n", t->pos.line, t->pos.col);
        }
        
        vec_pop(tokens);                                              // eat semi-colon

        struct astNode* node = astNode_create(&(struct astNode) { .type = AST_TYPE_STMT });
        node->stmt.returnStmt.exp = expNode;

        if (NULL != ast)
        {
            vec_push(ast, node);
        }

        return node;
    }
    else
    {
        parserErrorAndExit("unexpected command at line %d, column %d\n", SAFE_LIN(token), SAFE_COL(token));
    }
    
    return NULL;
}

// type_list = type ',' type_list | epsilon
static bool parse_type_list(struct vec* tl)
{
    bool res = true;

    struct token* token = vec_peekCurrent(tokens);

    if ((token != NULL) && (token->type = TOKEN_TYPE_TYPE))
    {
        vec_push(tl, token->sVal);
    }

    return res;
}

//function :: = "int" < identifier > "(" "void" ")" "{" < statements > "}"
static bool parse_function(struct vec* ast)
{
    bool res = false;
    struct token* token = vec_peekCurrent(tokens);
    struct astNode* node = NULL;

    if ((token != NULL) && (token->type == TOKEN_TYPE_TYPE))
    {
        char* fnctReturnType = token->sVal;
        vec_pop(tokens);                                           // eat type

        token = vec_peekCurrent(tokens);
        if ((token != NULL) && (token->type == TOKEN_TYPE_ID))
        {
            char* fnctName = token->sVal;
            vec_pop(tokens);                                       // eat name 

            parser_expect(TOKEN_TYPE_LPAREN, NULL, tokens);
            vec_pop(tokens);                                       // eat '('

            struct vec* type_list = NULL;
            vec_init(&type_list);


            if (parse_type_list(type_list))
            {
                vec_pop(tokens);                                   // eat last read token

                parser_expect(TOKEN_TYPE_RPAREN, NULL, tokens);    
                vec_pop(tokens);                                   // eat ')'

                node = astNode_create(&(struct astNode) { .type = AST_TYPE_FUNCTION });
                node->fnct.retType = fnctReturnType;
                node->fnct.name = fnctName;
                node->fnct.args = type_list;

                struct vec* stmt_list=NULL;
                vec_init(&stmt_list);
                
                do
                {
                    struct astNode* stmt = parse_statement(NULL);
                    vec_push(stmt_list, stmt);

                    token = vec_peekCurrent(tokens);
                } while ((NULL != token) && (token->type != TOKEN_TYPE_RCURLYB));

                vec_pop(tokens);                         // eat terminating semicolon
                node->fnct.stmt = stmt_list;             // add vector if statements to function node

                struct astNode* top = vec_peekCurrent(ast);
                vec_push(top->prog.fncts, node);
                
                res = true;
            }
            else
            {
                parserErrorAndExit("failed to parse argument list for function %s\n", fnctName);
            }
        }
        else
        {
            parserErrorAndExit("unexpected symbol at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
        }
    }
    else
    {
        parserErrorAndExit("unexpected symbol at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
    }

    return res;
}

// program ::= function
static bool parse_program()
{
    bool res = false;
    
    struct vec* ast = NULL;
    vec_init(&ast);

    struct astNode* node = astNode_create(&(struct astNode) { .type = AST_TYPE_PROGRAM });
    vec_push(ast, node);
    vec_setCurrentNdx(ast, 0);

    if (parse_function(ast))
    {
        printAST(ast);

        struct token* t = vec_peekCurrent(tokens);
        tok_print(t);

        if (NULL == t)                       // reached end of token stream
        {
            res = true;
        }
    }
    else
    { 
        parserErrorAndExit("[-] parse error, unexpected token at beginning\n");
    }

    return res;
}



bool parser_parse()
{
    bool res = false;

    vec_setCurrentNdx(tokens, 0);                                   // move to the first token
    res = parse_program();

    return res;
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

    if (NULL == name)                                     // no name given, based solely on type
    {
        if (t->type == type)
            res = true;
    }
    else
    {
        if ((t->type == type) && (strcmp(name, getTokenName(t)) == 0))
            res = true;
    }

    if(!res)
    {
        parserErrorAndExit("[-] parse error, unexpected token at line %d, column %d", t->pos.line, t->pos.col);
    }

    return res;
}

bool parser_statement()
{
    bool res = false;

    return res;
}

void parserErrorAndExit(const char* fmt, ...)
{
  char  buf[1024];
  
  va_list args;
  va_start(args, fmt);

  vsprintf(buf, fmt, args);
  va_end(args);
  
  fprintf(stderr, "[-] parser error: %s\n", buf);
  exit(ERR_PARSE_FAILED);
}
