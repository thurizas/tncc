#include <stdio.h>

#define max(x, y)								\
  if((x) > (y)) x								\
	else y				  
  /*
   * test three
   */
int main(void)
{
  int/*return value*/ ret = 0;
  printf("%d\n", max(5,6));

  return ret;
}
