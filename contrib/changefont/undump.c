#include <stdio.h>


int getbyte(char *ootje)
{
  int i;
  int mask=128;
  int result;
  
  result=0;
  for(i=0;i<8;i++)
  {
    if(ootje[i]!=' ') result|=mask;
    mask/=2;
  }
  return result;
}

void main()
{
  char instr[20];
  
  while(!feof(stdin))
  {
    fgets(instr,sizeof(instr),stdin);
    if(!feof(stdin)) putchar(getbyte(instr));
  }
}
