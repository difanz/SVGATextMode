#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "misc.h"
#include "vga_prg.h"
#include "messages.h"
#include "confdefs.h"

char *CommandName;
int debug_messages=FALSE;

#define NUM_BITS   8
char* int_to_bin(int num, int bits)
{
  static char binstr[sizeof(long int)+1];
  int i;
  
  for (i=0; i<bits; i++) binstr[i] = 0x30; /* char '0' */
  binstr[bits] = 0x00;
  
  for (i=0; i<bits; i++) binstr[bits-1-i] += ((num >> i) & 0x00000001);
  PDEBUG(("binstr = '%s'", binstr));
  return(binstr);
}

/***********************************************************************************************************/
 
int main (int argc, char* argv[])
{
  char tempstr[1024]="";
  int program_hardware=TRUE;
  int unlock_chipset=FALSE;
  int pipehex=FALSE;
  int pipe=FALSE;
  int regset = -1;
  char c;
  int tmpbyte=0;
  int regnum=0;
  int setreg = FALSE; /* if TRUE: "getVGAreg" function, if FALSE, "setVGAreg" function */
  char* commandfilename;
  int data=0;
  
 /*
  * See what action is required: read or write VGA register
  */
    
  CommandName = argv[0];
 
 /*
  * command-line argument parsing
  */

  while ((c = getopt (argc, argv, "ndhupxt:")) != EOF)
    switch (c)
    {
      case 'n': program_hardware=FALSE;
                break;
      case 'd': debug_messages=TRUE;
                break;
      case 'u': unlock_chipset = TRUE;
                break;
      case 'p': pipe = TRUE;
                break;
      case 'x': pipehex = TRUE;
                break;
      default: PERROR(("getopt returned unknown token '%c'.",c));
    }
    
#define GENDAC_INDEX         0x3C8
#define GENDAC_DATA          0x3C9

 if (program_hardware)
  {
     unsigned char tmp, tmp1;
     int vgaCRIndex = vgaIOBase + 4;
     int vgaCRReg = vgaIOBase + 5;


     get_VGA_io_perm(CS_S3);
     if (unlock_chipset) unlock(CS_S3); 
     
     /* set RS2 via CR55, yuck */
     outb(0x55, vgaCRIndex);
     tmp = inb(vgaCRReg) & 0xFC;
     outb(tmp | 0x01, vgaCRReg);
     tmp1 = inb(GENDAC_INDEX);

     outb(2, GENDAC_INDEX);
     printf("PLL data regs = 0x%2X, ", inb(GENDAC_DATA));
     printf("0x%2X\n", inb(GENDAC_DATA));
          
     printf("PLL command reg = 0x%2X\n", inb(DAC_MASK));

     /* Now clean up our mess */
     outb(tmp1, GENDAC_INDEX);
     outb(tmp, vgaCRReg);

  }
  return(0);
}
