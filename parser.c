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

static struct astNode* node = NULL;            // root of the AST
                                               // this will be deleted when codeGen is done with it.

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

struct astNode* parser_getAst()
{
    return node;
}

void parser_delAst()
{
    // TODO: delete ast here
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
        struct astNode* node = astNode_create(&(struct astNode) { .type = AST_TYPE_STMT });
        node->stmt.type = AST_STMT_TYPE_RETURN;

        vec_pop(tokens);                                               // eat 'return'
        struct astNode* expNode = parse_expression(NULL);
        if (!parser_expect(TOKEN_TYPE_SEMICOLON, NULL, tokens))
        {
            struct token* t = vec_peekCurrent(tokens);
            parserErrorAndExit("expected semicolon at line: %d, column %d\n", t->pos.line, t->pos.col);
        }
        
        vec_pop(tokens);                                              // eat semi-colon

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
static bool parse_function(struct astNode* fnctNode)
{
    bool res = false;
    struct token* token = vec_peekCurrent(tokens);
    //./struct astNode* node = NULL;

    struct vec* type_list = NULL;
    vec_init(&type_list);

    if (parse_type_list(type_list))
    {
        vec_pop(tokens);                                   

        if(parser_expect(TOKEN_TYPE_RPAREN, NULL, tokens))
        {
            vec_pop(tokens);                                   // eat ')'
            fnctNode->fnct.args = type_list;

            struct vec* stmt_list=NULL;
            vec_init(&stmt_list);
                
            do
            {
                struct astNode* stmt = parse_statement(NULL);
                vec_push(stmt_list, stmt);

                token = vec_peekCurrent(tokens);
            } while ((NULL != token) && (token->type != TOKEN_TYPE_RCURLYB));

            vec_pop(tokens);                         // eat terminating semicolon
            fnctNode->fnct.stmts = stmt_list;         // add vector if statements to function node
                
            res = true;
        }
        else
        {
            struct token* t = vec_peekCurrent(tokens);
            parserErrorAndExit("expected ')' at line %d, col %d\n", SAFE_LIN(t), SAFE_COL(t));
            }
        }
    else
    {
        parserErrorAndExit("unexpected symbol at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
    }
 
    // TODO : need to delete typelist here...
    return res;
}

// program ::= decl | fnct | typedef | struct |  union  | enum  
static bool parse_program()
{
    bool res = false;

    node = astNode_create(&(struct astNode) { .type = AST_TYPE_PROGRAM });
    vec_init(&node->prog.fncts);
    
    while (true)
    {
        struct token* t = vec_getCurrent(tokens);

        if ((t != NULL) && (t->type == TOKEN_TYPE_KEYWORD) && (strcmp("typedef", t->sVal) == 0))
        {
            // TODO - parse a typedef statement and add to AST
            //parse_typedef();
        }
        else if ((t != NULL) && (t->type == TOKEN_TYPE_KEYWORD) && (strcmp("struct", t->sVal) == 0))
        {
            // TODO - parse a structure definition and add to AST
            //parse_structure();
        }
        else if ((t != NULL) && (t->type == TOKEN_TYPE_KEYWORD) && (strcmp("union", t->sVal) == 0))
        {
            // TODO - parse an union definition and add to AST
            //parse_union();
        }
        else if ((t != NULL) && (t->type == TOKEN_TYPE_KEYWORD) && (strcmp("enum", t->sVal) == 0))
        {
            // TODO - parse an enumeration definition and add to AST
            //parse_enum();
        }
        else if ((t != NULL) && (t->type = TOKEN_TYPE_TYPE))
        {
            char* type = t->sVal;
            char* name = NULL;

            if (parser_expect(TOKEN_TYPE_ID, NULL, tokens))
            {
                t = vec_getCurrent(tokens);
                name = t->sVal;

                if (parser_expect(TOKEN_TYPE_LPAREN, NULL, tokens))
                {
                    vec_pop(tokens);                               // eat left paranthesis
                    struct astNode* fnctNode = astNode_create(&(struct astNode) { .type = AST_TYPE_FUNCTION, .flags=0x41414141, .pos.line=0x42424242, .pos.col = 0x43434343, .fnct.retType = type, .fnct.name=name});
                    if (parse_function(fnctNode))
                    {
                        vec_push(node->prog.fncts, fnctNode);
                    }
                    else
                    {
                        fprintf(stderr, "failed to parse function %s, line %d, col %d\n", name, t->pos.line, t->pos.col);
                        break;
                    }
                }
                else if(parser_expect(TOKEN_TYPE_SEMICOLON, NULL, tokens) || parser_expect(TOKEN_TYPE_COMMA,NULL, tokens))
                {
                    // TODO: parse variable definition
                    //parse_varDefinition();
                }
                else
                {
                    fprintf(stdout, "unexpected token at line %d, column %d\n", t->pos.line, t->pos.col);
                    break;
                }
            }
            else
            {
                fprintf(stdout, "unexpected token at line %d, column %d\n", t->pos.line, t->pos.col);
                break;
            }
        }
        else if (t == NULL) 
        {
            res = true;
            break;
        }
        else
        {
            fprintf(stdout, "unexpected token at line %d, column %d\n", t->pos.line, t->pos.col);
            break;
        }
    };

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
