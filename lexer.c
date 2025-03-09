#include "buffer.h"
#include "vector.h"
#include "node.h"
#include "lexer.h"
#include "common.h"
#include "util.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>


#include <stdbool.h>
#include <stdint.h>

static const char* types[] = { "void", "char", "short", "int", "long", "float", "double" };
static const char* keywords[] = { "return" };

static FILE* fp = NULL;
static struct buffer* buf = NULL;                // buffer to hold preprocessed file
static struct vec* tokens = NULL;                // vector to hold all the tokens

static bool lexer_isType(struct buffer*);
static bool lexer_isKeywork(struct buffer*);

bool lexer_init(const char* name, uint8_t flags)
{
  bool res = false;
  uint16_t state = LINE_BEGIN;                            // bit field denotating state of lexer
  
  fprintf(stderr, "lexing file: %s flags are: 0x%02x\n", name, flags);

  fp = fopen(name, "r");
  if(fp != NULL)
  {
	char ch;
	//struct buffer* buf = NULL;
	buf_init(&buf);

	while((ch = fgetc(fp)) != EOF)
	{
	  switch(ch)
	  {
		case '\n' :
		  if(state & LINE_COMMENT) state ^= LINE_COMMENT; // turn of line comment state
		  state ^= LINE_BEGIN;                            // starting a new line, reset beginning of line flag
		  buf_append(buf, ch);
		  break;

		case '/' :                                        // dealing with an '/' 
		{
		  if(state & LINE_BEGIN) state ^= LINE_BEGIN;
		  char nextCh = fgetc(fp);

		  if(nextCh == '/')                               // dealing with a '//' character sequence
		  {
			state = state | LINE_COMMENT;
		  }
		  else if(nextCh == '*')                          // dealing with a '/*'
		  {
			state = state | BLCK_COMMENT;
		  }
		  else                                            // not the characters you are searching for....
		  {
			ungetc(nextCh, fp);                           // put next character back in the stream
			if (!((state & LINE_COMMENT) || (state & BLCK_COMMENT))) buf_append(buf, ch); // add the slash to the output buffer
		  }
		} 

		break;

		case '\\' :                                       // escape character
		{
		  char nextCh = fgetc(fp);
		  if(nextCh != '\n')                              // not line continuation
		  {
			ungetc(nextCh, fp);
			buf_append(buf, ch);
		  }
		}
		break;

		case '*' :
		{
		  char nextCh = fgetc(fp);
		  if(state & LINE_BEGIN) state ^= LINE_BEGIN;
		  
		  if(nextCh == '/')                               // dealing with a '*/' character sequence 
		  {
		
			state ^= BLCK_COMMENT;
			continue;
		  }
		  else
		  {
			ungetc(nextCh, fp);
			if(!((state & LINE_COMMENT) || (state & BLCK_COMMENT))) buf_append(buf, ch);
		  }
		}

		break;

		case ' ':                                         // dealing with a space
		  if(state & LINE_BEGIN) continue;
		  if(buf_at(buf, buf_len(buf)-1) == ' ') continue;
		  buf_append(buf, ch);
		  break;

		case '\t':                                        // replace a tab with a single space
		{
		  if(buf_at(buf, buf_len(buf)-1) != ' ')
			buf_append(buf, ' ');
		}
		  break;

		default :

		  if((state & LINE_COMMENT) || (state & BLCK_COMMENT))  // do not add comment 
			continue;

		  if(state & LINE_BEGIN) state ^= LINE_BEGIN;
		  
		  buf_append(buf, ch);
		  break;
	  }
	}

	buf_print(buf);
	res = true;
	fclose(fp);
  }
  else
  {	
	char* msg = malloc(250 * sizeof(char));
	sprintf(msg, "Failed to open file %s, error %d", name, errno);
	exitFailure(msg, ERR_LEX_FNOTFOUND);
  }

  return res;
}


void lexer_deinit()
{
  buf_free(&buf);
}

struct vec* lexer_getTokens()
{
	return tokens;
}

// at this point the input file has been read into an interal buffer (struct buffer buf) and has been preprocessed 
// so (1) tabs have been replaced with a single space, (2) sequences of spaces have been compressed to a single 
// space, (3) that comments have been removed, (4) multiple lines ending with a line-continuation character have 
// been converted into a single line.
bool lexer_lex()
{
	bool  res = true;
	static uint32_t line = 1;
	static uint32_t col = 1;

	vec_init(&tokens);

	uint32_t cntChars = buf_len(buf);

	for (uint32_t ndx = 0; ndx < cntChars; ndx++)
	{
	  char ch = buf_at(buf, ndx);
	  col += 1;
	  switch (ch)
	  {
		case '\n':
		  line += 1;
		  col = 1;
		  break;

		case '\r':
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
		  struct buffer* temp = NULL;
		  buf_init(&temp);
		  buf_append(temp, ch);            
		  
		  while ((ndx+1 < cntChars) && isdigit(buf_at(buf, ndx + 1)))
		  {
			ndx += 1;
			ch = buf_at(buf, ndx);
			buf_append(temp, ch);
			col += 1;
		  }

		  struct token* t = NULL;
		  if (NULL != (t = malloc(sizeof(struct token))))
		  {
			t->type = TOKEN_TYPE_INT;
			t->pos.line = line;
			t->pos.col = col;
			t->iVal = atoi(buf_data(temp));
			vec_enqueue(tokens, 0, t);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }

		  buf_free(&temp);
		}
		break;

		case '(':
		{
		  struct token* token = NULL;
		  if (NULL != (token = malloc(sizeof(struct token))))
		  {
			token->pos.line = line;
			token->pos.col = col;
			token->type = TOKEN_TYPE_LPAREN;
			token->cVal = '(';
			vec_enqueue(tokens, 0, token);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		}
		break;

		case ')' :
		{
		  struct token* token = NULL;
		  if(NULL != (token = malloc(sizeof(struct token))))
		  {
			token->pos.line = line;
			token->pos.col = col;
			token->type = TOKEN_TYPE_RPAREN;
			token->cVal = ')';
			vec_enqueue(tokens, 0, token);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		}
		break;
		
		case '{':
		{
		  struct token* token = NULL;
		  if (NULL != (token = malloc(sizeof(struct token))))
		  {
			token->pos.line = line;
			token->pos.col = col;
			token->type = TOKEN_TYPE_LCURLYB;
			token->cVal = '{';
			vec_enqueue(tokens, 0, token);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		}
		break;

		case '}':
		{
		  struct token* token = NULL;
		  if (NULL != (token = malloc(sizeof(struct token))))
		  {
			token->pos.line = line;
			token->pos.col = col;
			token->type = TOKEN_TYPE_RCURLYB;
			token->cVal = '}';
			vec_enqueue(tokens, 0, token);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		}
		break;
		
		case ';':
		{
		  struct token* t = NULL;
		  if (NULL != (t = malloc(sizeof(struct token))))
		  {
			t->type = TOKEN_TYPE_SEMICOLON;
			t->pos.line = line;
			t->pos.col = col;
			t->cVal = ';';
			vec_enqueue(tokens, 0, t);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		}
		break;

		case '+':
		{
			struct token* t = NULL;
			t = tncc_calloc(1, sizeof(struct token));
			t->pos.line = line;
			t->pos.col = col;
			if ((ndx + 1 < cntChars) && buf_at(buf, ndx + 1) == '=')
			{
				t->type = TOKEN_TYPE_PLUS_EQUALS;
				t->sVal = tncc_calloc(3, sizeof(unsigned char));
				t->sVal[0] = '+'; t->sVal[1] = '=';
				ndx++;
			}
			else
			{
				t->type = TOKEN_TYPE_PLUS;
				t->cVal = '+';
			}

			vec_enqueue(tokens, 0, t);
		}
		break;

		case '-' :
		{
			struct token* t = NULL;
			t = tncc_calloc(1, sizeof(struct token));
			t->pos.line = line;
			t->pos.col = col;

			char nextCh = buf_peekAt(buf, ndx);      //  peak at next token

			if (nextCh == '-')                       // got '--'
			{
				t->type = TOKEN_TYPE_DECREMENT;
				t->sVal = tncc_calloc(1, 3 * sizeof(char));
				t->sVal[0] = '-'; t->sVal[1] = '-';
				ndx = ndx + 1;                       // eat second glyph in operator 
			}
			else
			{ 
				t->type = TOKEN_TYPE_MINUS;
				t->cVal = '-';
			}

			vec_enqueue(tokens, 0, t);
		}
		break;

		case '*':         // TODO : need to differentiate between 3*5 (mult) and int* (type)
		{
			struct token* t = NULL;
			t = tncc_calloc(1, sizeof(struct token));
			t->type = TOKEN_TYPE_MULTI;
			t->pos.line = line;
			t->pos.col = col;
			t->cVal = '*';
			vec_enqueue(tokens, 0, t);
		}
		break;

		case '/':
		{
			struct token* t = NULL;
			t = tncc_calloc(1, sizeof(struct token));
			t->type = TOKEN_TYPE_DIV;
			t->pos.line = line;
			t->pos.col = col;
			t->cVal = '/';
			vec_enqueue(tokens, 0, t);
		}
		break;

		case '%':
		{
			struct token* t = NULL;
			t = tncc_calloc(1, sizeof(struct token));
			t->type = TOKEN_TYPE_MOD;
			t->pos.line = line;
			t->pos.col = col;
			t->cVal = '%';
			vec_enqueue(tokens, 0, t);
		}
		break;

		case ',':
		{
		  struct token* t = NULL;
		  if (NULL != (t = malloc(sizeof(struct token))))
		  {
			t->type = TOKEN_TYPE_COMMA;
			t->pos.line = line;
			t->pos.col = col;
			t->cVal = ',';
			vec_enqueue(tokens, 0, t);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		}
		break;

		case '~' :
		{
			struct token* t = NULL;
			if (NULL != (t = malloc(sizeof(struct token))))
			{
				t->type = TOKEN_TYPE_TILDA;
				t->pos.line = line;
				t->pos.col = col;
				t->cVal = '~';
				vec_enqueue(tokens, 0, t);
			}
			else
			{
				exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
			}
		}
		break;
		
		case '_':                          // start of an identifier
		case 'a':                          // identifer start with a letter or underscore
		case 'A':
		case 'b':
		case 'B':
		case 'c':
		case 'C':
		case 'd':
		case 'D':
		case 'e':
		case 'E':
		case 'f':
		case 'F':
		case 'g':
		case 'G':
		case 'h':
		case 'H':
		case 'i':
		case 'I':
		case 'j':
		case 'J':
		case 'k':
		case 'K':
		case 'l':
		case 'L':
		case 'm':
		case 'M':
		case 'n':
		case 'N':
		case 'o':
		case 'O':
		case 'p':
		case 'P':
		case 'q':
		case 'Q':
		case 'r':
		case 'R':
		case 's':
		case 'S':
		case 't':
		case 'T':
		case 'u':
		case 'U':
		case 'v':
		case 'V':
		case 'w':
		case 'W':
		case 'x':
		case 'X':
		case 'y':
		case 'Y':
		case 'z':
		case 'Z':
		{				
		  struct buffer* temp = NULL;
		  uint32_t start = col;
		  buf_init(&temp);
		  
		  buf_append(temp, ch);               // append first letter
		  
		  while ((ndx+1 < cntChars) && isValidIdentifier(buf_at(buf, ndx+1)))
		  {
			ndx += 1;                        // increment counter
			ch = buf_at(buf, ndx);
			buf_append(temp, ch);
			col += 1;
		  }
		  
		  struct token* t = NULL;
		  if (NULL != (t = malloc(sizeof(struct token))))
		  {
			t->type = TOKEN_TYPE_ID;
			t->pos.line = line;
			t->pos.col = start;
			t->sVal = malloc((buf_len(temp) + 1) * sizeof(char));
			memset((void*)t->sVal, '\0', buf_len(temp) + 1);
			memcpy(t->sVal, buf_data(temp), buf_len(temp));
			
			if (lexer_isType(temp)) { t->type = TOKEN_TYPE_TYPE;  }
			if (lexer_isKeywork(temp)) { t->type = TOKEN_TYPE_KEYWORD; }

			vec_enqueue(tokens, (int)strlen(t->sVal)+1, t);
		  }
		  else
		  {
			exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
		  }
		  
		  buf_free(&temp);
		}
		break;
		
		case ' ':
		  ;
		  break;
		  
		default:
		  fprintf(stderr, "[?] unknown glyph: %c(%d)at line %d, column %d\n", ch, (int)ch, line, col);
		  res = false;
		  break;
	  }
	  
	}

	return res;	
}


static bool lexer_isType(struct buffer* token)
{
	bool res = false;

	uint32_t cntTypes = sizeof(types) / sizeof(types[0]);
	for (uint32_t ndx = 0; ndx < cntTypes; ndx++)
	{
		if (strcmp(types[ndx], buf_data(token)) == 0)
		{
			res = true;
			break;
		}
	}

	return res;
}


static bool lexer_isKeywork(struct buffer* token)
{
	bool res = false;

	uint32_t cntKeywords = sizeof(keywords) / sizeof(keywords[0]);
	for (uint32_t ndx = 0; ndx < cntKeywords; ndx++)
	{
		if (strcmp(keywords[ndx], buf_data(token)) == 0)
		{
			res = true;
			break;
		}
	}

	return res;
}

// prints token vector to a file 
void lexer_print(const char* name, const char* ext)
{
	FILE* outfp = NULL;
	char* filename = NULL;

	size_t len = strlen(name) + strlen(ext) + 2;     // one extra for period, one extra for null-terminator

	filename = calloc(1, len * sizeof(char));
	strcat(strcpy(filename, name), ext);

	if (filename != NULL)
	{
		outfp = fopen(filename, "w");
		if (fp != NULL)
		{
			if ((tokens->head != NULL) && (tokens->tail != NULL))
			{
				char line[512];                        // buffer for a line
				struct node* node = tokens->head;
				do
				{
					struct token* t = (struct token*)node->data;

					memset((void*)line, '\0', 512);    // clear line buffer

					sprintf(line, "{ token type: %i, pos: (%i, %i), value:", t->type, t->pos.line, t->pos.col);
					size_t len = strlen(line);
					if (t->type == TOKEN_TYPE_INT) sprintf(&line[strlen(line)], "%d", t->iVal);
					else if (t->type == TOKEN_TYPE_TYPE) sprintf(&line[strlen(line)], "%s", t->sVal);
					else if (t->type == TOKEN_TYPE_KEYWORD) sprintf(&line[strlen(line)], "%s", t->sVal);
					else if (t->type == TOKEN_TYPE_ID) sprintf(&line[strlen(line)], "%s", t->sVal);
					else if (t->type == TOKEN_TYPE_DECREMENT) sprintf(&line[strlen(line)], "%s", t->sVal);
					else sprintf(&line[strlen(line)], "%c", t->cVal);
					sprintf(&line[strlen(line)], "  }\n");

					fwrite(line, sizeof(char), strlen(line), outfp);

					node = node->flink;
				} while (node != NULL);
			}
			else
			{
				fprintf(stderr, "[-] no token data to output to file\n");
			}


			fclose(outfp);
		}
		else
		{
			fprintf(stderr, "failed to open lexer output file %s, error is: %d\n", filename, errno);
		}

		free(filename);
	}
	else
	{
		fprintf(stderr, "failed to create buffer for lexer output file");
	}
}
