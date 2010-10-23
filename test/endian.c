#include <stdlib.h>
#include <stdio.h>

typedef unsigned char byte;

int main() 
{
  unsigned long i,a=0,b=0,c=0;
  int ret = 0;

  for(i=0;i<sizeof(unsigned long);i++) {
    ((byte *)&a)[i] = i;
    b<<=8;
    b |= i;
    c |= i << (i*8);
  }
  if     (a == b) printf("BIG_ENDIAN\n");
  else if(a == c) printf("LITTLE_ENDIAN\n");
  else{ ret = -1; printf("BAD_ENDIAN\n"); }

  return ret;
}
