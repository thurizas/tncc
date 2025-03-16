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

/*
 * Grammar:
 * 
 * <program> ::= <function>
 * <function> ::= 'int' <id> '(' <arg_list> ')' '{' <statement_list> '}'
 * <arg_list> ::= <type>[',' <arg_list>] | empty
 * <statement_list> ::= statement ';'[<statement_list>] | empty
 * <statement> ::= 'return' <exp> ';' 
 * <exp> ::= <factor> | <exp> <binop> <exp>
 * <factor> ::= <int> | <unop> <factor> | '(' <exp> ')'
 * <unop> ::= '~' | '-'
 * <binop> ::= '+' | '-' | '*' | '/' | '%'
 * <type> ::= 'void' | 'int'
 * <id> ::= [_a-zA-Z][_a-zA-Z0-9]*
 * <int> ::= [0-9]+
 */

static struct vec* tokens;                     // vector of tokens we are working with
static uint32_t flags;                         // flags controlling operation

static struct astNode* node = NULL;            // root of the AST
                                               // this will be deleted when codeGen is done with it.

enum assoc {RIGHT_TO_LEFT=0, LEFT_TO_RIGHT=1};
 static struct prec
{
    char    op[3];
    uint8_t prec;
    uint8_t assoc;
}prec[] = { {{'*','\0','\0'}, 50, LEFT_TO_RIGHT},
            {{'/','\0','\0'}, 50, LEFT_TO_RIGHT},
            {{'%','\0','\0'}, 50, LEFT_TO_RIGHT},
            {{'+','\0','\0'}, 45, LEFT_TO_RIGHT},
            {{'-','\0','\0'}, 45, LEFT_TO_RIGHT},
            {{'<', '<','\0'}, 40, LEFT_TO_RIGHT},
            {{'>', '>','\0'}, 40, LEFT_TO_RIGHT},
            {{'<','\0','\0'}, 35, LEFT_TO_RIGHT},
            {{'<','=' ,'\0'}, 35, LEFT_TO_RIGHT},
            {{'>','\0','\0'}, 35, LEFT_TO_RIGHT},
            {{'>','=' ,'\0'}, 35, LEFT_TO_RIGHT},
            {{'=','=' ,'\0'}, 30, LEFT_TO_RIGHT},
            {{'!','=' ,'\0'}, 30, LEFT_TO_RIGHT},
            {{'&','\0','\0'}, 25, LEFT_TO_RIGHT},
            {{'^','\0','\0'}, 24, LEFT_TO_RIGHT},
            {{'|','\0','\0'}, 23, LEFT_TO_RIGHT},
            {{'&','&' ,'\0'}, 20, LEFT_TO_RIGHT},
            {{'|','|' ,'\0'}, 19, LEFT_TO_RIGHT},
            {{'?', ':','\0'}, 15, RIGHT_TO_LEFT},
            {{'=','\0','\0'}, 10, RIGHT_TO_LEFT},
            {{'+','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'-','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'*','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'/','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'%','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'<','<' ,'=' }, 10, RIGHT_TO_LEFT},
            {{'>','>' ,'=' }, 10, RIGHT_TO_LEFT},
            {{'&','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'^','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{'|','=' ,'\0'}, 10, RIGHT_TO_LEFT},
            {{',','\0','\0'}, 5, LEFT_TO_RIGHT}};

static bool parser_expect(int, const char*, struct vec*);
static struct astNode* parse_exp(struct vec* ast, int minPrec);
static struct astNode* parse_factor(struct vec* ast);


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
    astNode_delete(node);
}

static bool isUnaryOp(struct token* token)
{
    return ((token->type == TOKEN_TYPE_TILDA) || (token->type == TOKEN_TYPE_MINUS));
}

static bool isBinOp(struct token* token)
{
    return ((token->type == TOKEN_TYPE_PLUS) || (token->type == TOKEN_TYPE_MINUS) || (token->type == TOKEN_TYPE_MULTI) ||
            (token->type == TOKEN_TYPE_DIV) || (token->type == TOKEN_TYPE_MOD));
}

static int getPrecedence(const char* op)
{
    int cntOps = sizeof(prec) / sizeof(prec[0]);
    int ret = -1;

    for (int ndx = 0; ndx < cntOps; ndx++)
    {
        if (strcmp(op, prec[ndx].op))
        {
            ret = prec[ndx].prec;
            break;
        }
    }

    return ret;
}

////<exp> ::= <int> | <unop> <exp> | '(' <exp> ')'
//static struct astNode* parse_expression(struct vec* ast)
//{
//    struct astNode* node = NULL;
//
//    struct token* token = vec_getCurrent(tokens);
//
//    if ((token != NULL) && (token->type == TOKEN_TYPE_INT))
//    {
//        node = astNode_create(&(struct astNode){.type = AST_TYPE_INTVAL, .iVal = token->iVal});
//        if (NULL != ast)
//        {
//            vec_enqueue(ast, sizeof(struct astNode), node);
//        }
//
//        vec_pop(tokens);                                             // remove the numeric literal node
//        return node;
//    }
//    else if ((token != NULL) && (token->type == TOKEN_TYPE_TILDA))   // unon exp
//    {
//        node = astNode_create(&(struct astNode) { .type = AST_TYPE_EXPR, .stmt.type = AST_TYPE_UNOP, .iVal = token->iVal });
//        vec_pop(tokens);
//        struct astNode* expNode = parse_expression(NULL);
//        node->exp.op = tncc_calloc(3, sizeof(char));
//        strcpy(node->exp.op,"~");
//        node->exp.left = expNode;
//
//        if (NULL != ast)
//        {
//            vec_enqueue(ast, sizeof(struct astNode), node);
//        }
//
//        return node;
//    }
//    else if ((token != NULL) && (token->type == TOKEN_TYPE_MINUS))
//    {
//        node = astNode_create(&(struct astNode) { .type = AST_TYPE_EXPR, .stmt.type = AST_TYPE_UNOP, .iVal = token->iVal });
//        vec_pop(tokens);
//        struct astNode* expNode = parse_expression(NULL);
//        node->exp.op = tncc_calloc(3, sizeof(char));
//        strcpy(node->exp.op, "-");
//        node->exp.left = expNode;
//
//        if (NULL != ast)
//        {
//            vec_enqueue(ast, sizeof(struct astNode), node);
//        }
//
//        return node;
//    }
//    else if ((token != NULL) && (token->type == TOKEN_TYPE_LPAREN))
//    {
//        node = astNode_create(&(struct astNode) { .type = AST_TYPE_EXPR });
//        vec_pop(tokens);                                                     // eat left paran.
//        struct astNode* expNode = parse_expression(NULL);
//        if (parser_expect(TOKEN_TYPE_RPAREN, NULL, tokens))
//        {
//            vec_pop(tokens);                                                 // eat right paran.
//            return expNode;
//        }
//        else
//        {
//            parserErrorAndExit("unmatched '(' found at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
//        }
//    }
//    else
//    {
//        parserErrorAndExit("Unknown or unexpected token at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
//    }
//
//    return node;
//}

// <int> :: = [0 - 9] +
static struct astNode* parse_int(struct vec* ast)
{
    struct astNode* intNode = astNode_create(&(struct astNode) { .type = AST_TYPE_INTVAL, .iVal = ((struct token*)vec_getCurrent(tokens))->iVal });
    return intNode;
}

// unary operators ::= ~ | - 
void parse_unop(char* op)
{
    struct token* token = vec_getCurrent(tokens);
    memset((void*)op, '\0', 3);

    if (token->type == TOKEN_TYPE_TILDA) op[0] = '~';
    else if (token->type == TOKEN_TYPE_MINUS) op[0] = '-';
    else op[0] = '?';
}

// binary operators ::= - | + | * | / | % 
// convert one or two glyph operators in to string values and store in token->sVal
// NOTE: when supporting to glyph operators, +=, <=, or << let the lexer check and do this logic
// NOTE: these multi-glyph operators are not couned as binay operators
bool parse_binop(char* op)
{
    bool res = true;

    struct token* token = vec_getCurrent(tokens);
    memset((void*)op, '\0', 3);

    if (token->type == TOKEN_TYPE_PLUS) op[0] = '+';
    else if (token->type == TOKEN_TYPE_MINUS) op[0] = '-';
    else if (token->type == TOKEN_TYPE_MULTI) op[0] = '*';
    else if (token->type == TOKEN_TYPE_DIV) op[0] = '/';
    else if (token->type == TOKEN_TYPE_MOD) op[0] = '%';
    else { op[0] = '?'; res = false;}

    return res;
}

// <factor> :: = <int> | <unop> <factor> | '(' < exp > ')'
static struct astNode* parse_factor(struct vec* ast)
{
    struct token* curToken = vec_getCurrent(tokens);
    if (curToken->type == TOKEN_TYPE_INT)
    {
        struct astNode* intNode = astNode_create(&(struct astNode) { .type = AST_TYPE_INTVAL, .iVal = ((struct token*)vec_getCurrent(tokens))->iVal });
        vec_pop(tokens);
        return intNode;
    }
    else if (isUnaryOp(curToken))
    {
        char op[3] = { '\0' };
        parse_unop(op);                                 // move to next token
        vec_pop(tokens);                                // eat unary operator
        struct astNode* innerExp = parse_factor(ast);

        struct astNode* uniNode = astNode_create(&(struct astNode) { .type = AST_TYPE_EXPR, .exp = innerExp });
        strcpy(uniNode->exp.op, op);
        return uniNode;
    }
    else if (curToken->type == TOKEN_TYPE_LPAREN)
    {
        vec_pop(tokens);                                // eat left-paranthesis
        
        struct astNode* inner_exp = parse_exp(ast, 0);
        if (parser_expect(TOKEN_TYPE_RPAREN, NULL, tokens))
        {
            vec_pop(tokens);                            // eat right-paranthesis
            return inner_exp;
        }
        else
            parserErrorAndExit("expected a right parenthesis, found token type%d\n", ((struct token*)vec_getCurrent(tokens))->type);
    }
    else
    {
        parserErrorAndExit("malformed expression\n");
    }
}

// <exp> :: = <factor> | <exp> <binop> <exp>
static struct astNode* parse_exp(struct vec* ast, int minPrec)
{
    struct token* temp = vec_getCurrent(tokens);
    struct astNode* left = parse_factor(ast);
    struct astNode* exp = NULL;
    struct token* nextToken = vec_getCurrent(tokens);
    char* op = tncc_calloc(3, sizeof(unsigned char));

    while (isBinOp(nextToken) && parse_binop(op) && getPrecedence(op) >= minPrec)
    {
        vec_pop(tokens);                                   // eat binary operator
        struct astNode* right = parse_exp(ast, getPrecedence(op) + 1);
        exp = astNode_create(&(struct astNode) { .type = AST_TYPE_EXPR, .exp.left = left, .exp.right = right });
        strncpy(exp->exp.op, op, 3);
        nextToken = vec_peekNext(tokens);
        return exp;
    }

    return left;
}

// <statement> :: = "return" < exp > ";"
static struct astNode* parse_statement(struct vec* ast)
{
    // eat optional currely brace if present
    if (((struct token*)vec_getCurrent(tokens))->type == TOKEN_TYPE_LCURLYB)
        vec_pop(tokens);

    struct token* token = vec_getCurrent(tokens);                      // get the command

    if ((token != NULL) && (token->type = TOKEN_TYPE_KEYWORD) && (strcmp(token->sVal, "return") == 0))
    {        
        struct astNode* node = astNode_create(&(struct astNode) { .type = AST_TYPE_STMT });
        node->stmt.type = AST_STMT_TYPE_RETURN;

        vec_pop(tokens);                                               // eat 'return'
        struct astNode* expNode = parse_exp(NULL, 0);
        if (!parser_expect(TOKEN_TYPE_SEMICOLON, NULL, tokens))
        {
            struct token* t = vec_getCurrent(tokens);
            parserErrorAndExit("expected semicolon at line: %d, column %d\n", t->pos.line, t->pos.col);
        }
        
        vec_pop(tokens);                                              // eat semi-colon

        node->stmt.returnStmt.exp = expNode;

        if (NULL != ast)
        {
            vec_enqueue(ast, sizeof(struct astNode), node);
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

    struct token* token = vec_getCurrent(tokens);

    if ((token != NULL) && (token->type = TOKEN_TYPE_TYPE))
    {
        vec_enqueue(tl, (int)strlen(token->sVal), token->sVal);
    }

    return res;
}

// function :: = "int" < identifier > "(" "void" ")" "{" < statements > "}"
static bool parse_function(struct astNode* fnctNode)
{
    bool res = false;
    struct token* token = vec_getCurrent(tokens);

    struct vec* type_list = NULL;
    vec_init(&type_list); 

    if (parse_type_list(type_list))
    {
        vec_pop(tokens);                                   

        if(parser_expect(TOKEN_TYPE_RPAREN, NULL, tokens))
        {
            vec_pop(tokens);                            // eat ')'
            vec_init(&(fnctNode->fnct.args));
            vec_copy(fnctNode->fnct.args, type_list);

            struct vec* stmt_list=NULL;
            vec_init(&stmt_list); 
                
            do
            {
                struct astNode* stmt = parse_statement(NULL);
                vec_enqueue(stmt_list, sizeof(struct astNode), stmt); 

                token = vec_getCurrent(tokens);
            } while ((NULL != token) && (token->type != TOKEN_TYPE_RCURLYB));

            vec_pop(tokens);                          // eat terminating semicolon
            fnctNode->fnct.stmts = stmt_list;         // add vector of statements to function node
                
            res = true;
        }
        else
        {
            struct token* t = vec_getCurrent(tokens);
            parserErrorAndExit("expected ')' at line %d, col %d\n", SAFE_LIN(t), SAFE_COL(t));
            }
        }
    else
    {
        parserErrorAndExit("unexpected symbol at line %d, col %d\n", SAFE_LIN(token), SAFE_COL(token));
    }
 
    if (type_list != NULL) vec_free(type_list);
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
            char* type = NULL;
            char* name = NULL;

            type = tncc_calloc(strlen(t->sVal) + 1, sizeof(char));
            memcpy(type, t->sVal, strlen(t->sVal));

            vec_pop(tokens);                // eat return type token
            if (parser_expect(TOKEN_TYPE_ID, NULL, tokens))
            {
                t = vec_getCurrent(tokens);
                name = tncc_calloc(strlen(t->sVal) + 1, sizeof(char));
                memcpy(name, t->sVal, strlen(t->sVal));

                vec_pop(tokens);            // eat function name token
                if (parser_expect(TOKEN_TYPE_LPAREN, NULL, tokens))
                {
                    vec_pop(tokens);                               // eat left paranthesis
                    struct astNode* fnctNode = astNode_create(&(struct astNode) { .type = AST_TYPE_FUNCTION, .flags=0x41414141, .pos.line=0x42424242, .pos.col = 0x43434343, .fnct.retType = type, .fnct.name=name});
                    if (parse_function(fnctNode))
                    {
                        vec_enqueue(node->prog.fncts, sizeof(struct astNode), fnctNode); 
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

    struct token* t = vec_getCurrent(tokens);            // peek at current token
    
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
