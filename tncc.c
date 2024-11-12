
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>

#ifdef __WIN32
#include "XGetopt.h"
#else
#include <unistd.h>
extern int getopt(int, char**, const char*);
extern char* optarg;
extern int optind;
#endif


#include "common.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"

static const char* DEFNAME = "a.out";

void showVersion(const char*);
void showHelp(const char*);

int main(int argc, char** argv)
{
  int choice = -1;
  int res = 0;
  uint8_t flags = FLAGS_ALL;
  char* outName = NULL;
  char* inName = NULL;

  for (int ndx = 0; ndx < argc; ndx++)
  {
	  fprintf(stdout, "argument[%d]: %s\n", ndx, argv[ndx]);
  }

  while(-1 != (choice = getopt(argc, argv, "dhvo:s:")))
  {
	switch(choice)
	{
	  case 'd':
		flags = flags | FLAGS_DEBUG;
		break;
		
	  case 'o':
		if(outName != NULL) free(outName);

		if (NULL != (outName = malloc(sizeof(char) * (strlen(optarg) + 1))))
		{
			memset((void*)outName, '\0', strlen(optarg) + 1);
			strcpy(outName, optarg);
		}
		else
		{
			fprintf(stderr, "[-] memory allocation error, failed to allocate space for output file name");
			res = -1;
			goto EXIT;
		}
		break;

	  case 's':
	  {
		char step = optarg[0];
		switch(step)
		{
		  case 'l':
			flags = flags ^ FLAGS_LEX;
			break;
		  case 'p':
			flags = flags ^ FLAGS_PAR;
			break;
		  case 'c':
			flags = flags ^ FLAGS_CODEGEN;
			break;
		}
	  }
	  break;

	  case 'v':
		showVersion(argv[0]);
		break;

	  case '?':
	  case 'h':
		showHelp(argv[0]);
		break;
	}
   }
	
  // if no output name is given use default name
  if(outName == NULL)
  {
	  if (NULL != (outName = malloc(sizeof(char) * (strlen(DEFNAME) + 1))))
	  {
		memset((void*)outName, '\0', strlen(DEFNAME)+1);
		memcpy(outName, DEFNAME, strlen(DEFNAME));
	  }
	  else
	  {
		  fprintf(stderr, "[-] memory allocation error, failed to allocate memory for output file name\n");
		  res = -1;
		  goto EXIT;
	  }

  }

  // get input file name....
  size_t len = strlen(argv[optind]);
  if (NULL != (inName = malloc(sizeof(char) * (len + 1))))
  {
	memset((void*)inName, '\0', (len+1));
	memcpy(inName, argv[optind], len);
  }
  else
  {
	  fprintf(stderr, "[-] memory allocation error, failed to allocate memory for input file name");
	  res = -1;
	  goto EXIT;
  }


  if(lexer_init(inName, flags))
  {
	if (lexer_lex())
	{
		struct vec* tokens = lexer_getTokens();
		vec_print(tokens, tok_print);                                         // print out tokens vector
		
		// check to see if we are required to parse, and if parser is initialized
		if (((flags & FLAGS_PAR) == FLAGS_PAR) && parser_init(tokens, flags))   
		{
		  if (parser_parse())
		  {
				// TODO: perform conversion of AST to IR representation
				// TODO: perform conversion of IR to asm represnetation
				// TODO: perform code emittion (i.e. asm to elf file)
		  }

		  parser_deinit();
		}
		else
		{
			fprintf(stderr, "[-] failed to parse %s\n", inName);
		}

		tokens_clear(tokens);                                                 // destory vector of tokens
		vec_free(tokens);
	}
	
                                                     
	lexer_deinit();
  }
  else
  {
	fprintf(stderr, "failed to lex input file\n");
	res = ERR_LEX_FAILED;
  }

EXIT:
  if(inName != NULL) free(inName);
  if(outName != NULL) free(outName);
  
  return res;
}

void showVersion(const char* name)
{
  printf("%s - version %d.%d.%d\n", name, MAJOR, MINOR, PATCH);
  exit(0);
}


void showHelp(const char* name)
{
  printf("%s - a tiny, nearly complete C compiler\n", name);
  printf("usage %s [options] <input_file>\n", name);
  printf("\noptions:                       \n");
  printf("d                set debug flag             \n");
  printf("o <file>         use <file> as output file  \n");
  printf("s                only perform upto stage s  \n");
  printf("                 s can be l, p, or c        \n");
  printf("v                display version information\n");
  printf("h                display short help screen  \n");
}
		
