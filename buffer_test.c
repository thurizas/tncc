#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
  struct buffer* buf = NULL;

  buf_init(&buf);

  buf_print(buf);

  for(int ndx = 0; ndx < 26; ndx++)
	buf_append(buf, 'A' + ndx);

  buf_print(buf);
  
  fprintf(stdout, "[+] popping last character %c\n", buf_pop(buf));
  fprintf(stdout, "[+] popping last character %c\n", buf_pop(buf));
  fprintf(stdout, "[+] popping last character %c\n", buf_pop(buf));

  buf_print(buf);


  buf_free(&buf);
  
  buf_print(buf);


  return 0;
}
