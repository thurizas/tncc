#include "lexer.h"
#include "vector.h"
#include "buffer.h"
#include "common.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include <stdbool.h>
#include <stdint.h>


static FILE* fp = NULL;
static struct buffer* buf = NULL;                // buffer to hold preprocessed file




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

	// read file in character, perform some pre-processing of file
	
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
		  if(buf_at(buf, buf_len(buf)) == ' ') continue;
		  buf_append(buf, ch);
		  break;

		case '\t':                                        // replace a tab with a single space
		  buf_append(buf, ' ');
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
	buf_free(&buf);
	res = true;
	fclose(fp);
  }
  else
  {
	fprintf(stderr, "Failed to open file %s, error %d\n", name, errno);
  }

  return res;
}


void lexer_deinit()
{
  if(buf != NULL) free(buf);
}


void lexer_lex()
{

}


