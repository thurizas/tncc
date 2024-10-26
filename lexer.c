#include "lexer.h"
#include "vector.h"
#include "buffer.h"
#include "common.h"
#include "util.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>


#include <stdbool.h>
#include <stdint.h>


static FILE* fp = NULL;
static struct buffer* buf = NULL;                // buffer to hold preprocessed file
static struct vec* tokens = NULL;                // vector to hold all the tokens


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
			buf_append(buf, ch);                          // add the slash to the output buffer
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

// at this point the input file has been read into an interal buffer (struct buffer buf) and has been preprocessed 
// so (1) tabs have been replaced with a single space, (2) sequences of spaces have been compressed to a single 
// space, (3) that comments have been removed, (4) multiple lines ending with a line-continuation character have 
// been converted into a single line.
bool lexer_lex()
{
	bool  res = false;
	static uint32_t line = 1;
	static uint32_t col = 1;

	vec_init(&tokens);

	uint32_t cntChars = buf_len(buf);

	/*
	the file 
	int main(void)
	{
	    return 2;
	}
	                            "012345678901234 56 7890123456 78 
	is converted to the buffer: "int main(void)\n{\nreturn 2;\n}\n"
	*/
	for (uint32_t ndx = 0; ndx < cntChars; ndx++)
	{
		char ch = buf_at(buf, ndx);
		switch (ch)
		{
		    case '\n':
				line += 1;
				col = 1;
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
					vec_push(tokens, t);
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
					vec_push(tokens, token);
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
					vec_push(tokens, token);
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
					vec_push(tokens, token);
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
					vec_push(tokens, token);
				}
				else
				{
					exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
				}
			}

			case ';':
			{
				struct token* t = NULL;
				if (NULL != (t = malloc(sizeof(struct token))))
				{
					t->type = TOKEN_TYPE_SEMICOLON;
					t->pos.line = line;
					t->pos.col = col;
					t->cVal = ';';
					vec_push(tokens, t);
				}
				else
				{
					exitFailure("Failed to allocate storage for token", ERR_LEX_MEMORY);
				}
			}
			break;

			case '_':                          // start of an identifier
			case 'a':                          // identifer start with a letter or underscore
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':   
			{				
				struct buffer* temp = NULL;
				uint32_t start = col;
				buf_init(&temp);

				buf_append(temp, ch);               // append first letter

				while ((ndx+1 < cntChars) && isValidIdentifier(buf_at(buf, ndx+1)))
				{
					ndx += 1;                      // increment counter
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
					vec_push(tokens, t);
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
				fprintf(stderr, "[?] unknown glyph: %c(%d)\n", ch, (int)ch);
				break;
		}

	}

    vec_print(tokens, tok_print);
	return true;
	
}

void tok_print(void* data)
{
	struct token* t = (struct token*)data;

	fprintf(stdout, "{ token type: %i, pos: (%i, %i), value:", t->type, t->pos.line, t->pos.col);
	if (t->type == TOKEN_TYPE_INT) fprintf(stdout, "%d", t->iVal);
	else if (t->type == TOKEN_TYPE_ID) fprintf(stdout, "%s", t->sVal);
	else fprintf(stdout, "%c", t->cVal);
	fprintf(stdout, "  }\n");


}


