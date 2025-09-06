#include <stdio.h>


void dobyte(int byte)
{
  int i;
  int mask=128;
  for(i=0;i<8;i++)
  {
    if(byte & mask) putchar('O');
    else putchar(' ');
    mask/=2;
  }
}

void main()
{
  int c;
  while(!feof(stdin))
  {
    c=getchar();
    if(feof(stdin)) continue;
    dobyte(c);
    putchar('\n');
  }
}
