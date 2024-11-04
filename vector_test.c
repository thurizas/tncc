#include "vector.h"

#include <stdlib.h>
#include <stdio.h>

void printChar(void*);

int main(int argc, char** argv)
{
  struct vec* chars = NULL;

  if(NULL != (chars = malloc(sizeof(struct vec))))
  {
	vec_init(&chars);

	vec_print(chars, printChar);

	for(int ndx = 0; ndx < 10; ndx++)
	{
	  char* temp = malloc(sizeof(char));
	  *temp = 'A' + ndx;
	  vec_push(chars, temp);
	}
	

	vec_print(chars, printChar);

	//vec_free(chars);

  }
  else
  {
	fprintf(stderr, "[-] failed to allocate memory for list\n");
  }

  return 0;
}


void printChar(void* data)
{

}
