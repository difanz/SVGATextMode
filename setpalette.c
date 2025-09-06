/*  SVGATextMode -- An SVGA textmode manipulation/enhancement tool
 *
 *  Copyright (C) 1995-1998  Koen Gadeyne
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/***
 *** setpalette: palette set/get program
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#ifndef DOS
#  include <sys/kd.h>
#endif
#include "misc.h"
#include "vga_prg.h"
#include "messages.h"
#include "file_ops.h"
#include "string_ops.h"
#include "kversion.h"
#include "cfg_data.h"

extern unsigned char STD_PALETTE[64][3];


/* this will keep the compiler happy when compiling on < 1.3.3 systems */
#ifndef GIO_CMAP
#  define GIO_CMAP 0x4B70
#endif
#ifndef PIO_CMAP
#  define PIO_CMAP 0x4B71
#endif

char *CommandName;
bool debug_messages=FALSE;
bool setreg=FALSE;

int STM_Options=0;  /* just to keep the compiler happy */

/*
 * problem: kernel does not want VGA palette index, but ANSI index!
 * these are the differences:
 *
 * color:   VGA index:     ANSI index:
 *
 * black        0               0
 * blue         1               4
 * green        2               2
 * cyan         3               6
 * red          4               1
 * magenta      5               5
 * brown/yellow 6               3
 * white        7               7
 *
 * for high intensity versions, add 8 to both VGA index and ANSI index
 *
 * quite luckily, both VGA->ANSI table and ANSI->VGA table
 * turn out to be identical
 *
 * also, kernel wants red in first byte, then green and then blue.
 */
unsigned char color_table[] = { 0, 4, 2, 6, 1, 5, 3, 7,
                                8,12,10,14, 9,13,11,15 };

unsigned char kernel_palette[16][3];
bool eightbit=FALSE;

inline void printrgb(int index, int r, int g, int b, int hexdata)
{
  if (eightbit)
  {
    r<<=2; g<<=2; b<<=2;
  }
  if (hexdata)
    printf("0x%02x: 0x%02x 0x%02x 0x%02x\n",index,r,g,b);
  else
    printf("%03d: %03d %03d %03d\n",index,r,g,b);
}

void dumprgb(int index, int hexdata)
{
  int r,g,b;
  
  outb(index,DAC_STATUS);
  r=inb(DAC_DATA);
  g=inb(DAC_DATA);
  b=inb(DAC_DATA);
  printrgb(index, r, g, b, hexdata);
}  

void program_rgb(int index, int r, int g, int b)
{
  if (eightbit)
  {
    r>>=2; g>>=2; b>>=2;
  }
  WRITERGB(index, r, g, b);
  if (index < 16)
  {
    /* kernel wants red in first byte, then green and then blue. */
    kernel_palette[color_table[index]][0] = (unsigned char)r<<2;
    kernel_palette[color_table[index]][1] = (unsigned char)g<<2;
    kernel_palette[color_table[index]][2] = (unsigned char)b<<2;
  }
}


void usage(int setreg)
{
   if (setreg)
     PMESSAGE(("version %s. (c) 1995-1998 Koen Gadeyne.\n Usage: %s [options]  color_index  <R> <G> <B>\n\n\
     Options: -n  Don't program VGA hardware\n\
              -d  print debugging information\n\
              -x  input palette values in hex instead of decimal\n\
              -8  use 8-bit color definition (0..255) instead of 6-bit (0..63)\n\
              -s  load a standard VGA textmode palette.\n\
                   this is not necessarily the default system startup palette.\n\
                   (use `getpalette -s' to find out what palette this is\n\
              -k  do not tell kernel about color palette changes.\n\
                   Any post-1.3.2 kernel will then restore the old\n\
                   palette when switching consoles.\n\
              -h  print usage information\n\n\
     color_index: index number in the palette look-up table (0..255).\n\
                  or '-' to use standard input\n\
                    (Input format = '<index>: <R-value> <G-value> <B-value>')\n\
     R, G, B : color intensity for Red, Green or Blue (0..63).\n",
     VERSION, CommandName));
   else
     PMESSAGE(("version %s. (c) 1995-1998 Koen Gadeyne.\n Usage: %s [options] <color_index>\n\n\
     Options: -n  Don't program VGA hardware\n\
              -d  print debugging information\n\
              -x  output palette values in hex instead of decimal\n\
              -8  use 8-bit color definition (0..255) instead of 6-bit (0..63)\n\
              -s  print the built-in standard VGA textmode palette.\n\
                   this is the palette that will be programmed when running `setpalette -s'\n\
              -h  print usage information\n\n\
     color_index: index number in the palette look-up table (0..255).\n\
                    or 'all' to show all 256 entries\n",
     VERSION, CommandName));
}
 

int main (int argc, char* argv[])
{
  int index,r,g,b;
  char* commandfilename;
  bool program_hardware=TRUE;
  bool hexdata=FALSE;
  bool stdpal=FALSE;
  bool tellkernel=TRUE;
  char c;
#ifndef DOS
  int fd = -1;
  int kernel_can_do_cmap = FALSE;
#endif   
 /*
  * See what action is required: read or write VGA register
  */

  CommandName = argv[0];
  commandfilename = strrchr(CommandName, '/');
  if (commandfilename) commandfilename++;
  else commandfilename = CommandName;
  setreg = (!strncasecmp(commandfilename,"set",3));
 
 /*
  * command-line argument parsing
  */

  while ((c = getopt (argc, argv, "ndxskh8")) != EOF)
    switch (c)
    {
      case 'n': program_hardware=FALSE;
                break;
      case 'd': debug_messages=TRUE;
                break;
      case 'x': hexdata=TRUE;
                break;
      case '8': eightbit=TRUE;
                break;
      case 's': stdpal=TRUE;
                break;
      case 'k': tellkernel=FALSE;
                break;
      case 'h': usage(setreg);
                exit(0);
                break;
      case '?': usage(setreg);
                PERROR(("Bad option `-%c'\n",(char)optopt));
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }
    
  PVERSION;

  PDEBUG(("'%cetpalette' function selected through command name '%s'\n", (setreg) ? 's' : 'g', commandfilename));

  if (!stdpal) 
  {
    /* get color-index from commandline , if '-' use stdin */
    if (argc<optind+1) PERROR(("Missing color index (or `%s') on commandline\n", (setreg) ? "-" : "all" ));
  }

  if (!stdpal || setreg)
    if (program_hardware) get_VGA_io_perm(CS_VGA);

#ifndef DOS
  kernel_can_do_cmap = check_kernel_version(1,3,3,"GIO_CMAP/PIO_CMAP");
  
 /*
  * Start doing something useful
  */

  if (program_hardware && setreg && kernel_can_do_cmap && tellkernel)
  {
    fd = opentty( ConsoleDevice("0") );
    if (ioctl(fd, GIO_CMAP, kernel_palette))    /* this ioctl first popped up in kernel 1.3.3 */
    {
       perror("GIO_CMAP");
       PERROR(("Could not do GIO_CMAP on %s\n", ConsoleDevice("0")));
    }
  }
#endif

  if (stdpal)
  {
    if (setreg)
    {
      if (program_hardware)
      {
        for (index=0; index<64; index++)
          program_rgb(index,STD_PALETTE[index][0],STD_PALETTE[index][1],STD_PALETTE[index][2]);
        for (index=64; index<256; index++)
          program_rgb(index,0,0,0);
      }
    }
    else
    {
      if (program_hardware)
      {
        for(index=0;index<64;index++)
          printrgb(index, STD_PALETTE[index][0],STD_PALETTE[index][1],STD_PALETTE[index][2], hexdata);
        for (index=64; index<256; index++)
          printrgb(index,0,0,0, hexdata);
      }
    } 
  }
  else
  {
    if (setreg)
    {  /* setpalette */
    if(!strcmp(argv[optind],"-"))      /* use standard input */
      {
        char linebuf[1000];
        int lc=0;

        while(fgets(linebuf,999,stdin))
        {
          lc++;
          r=g=b=-1;
          if (hexdata)
            sscanf(linebuf,"0x%02x: 0x%02x 0x%02x 0x%02x",&index,&r,&g,&b);
          else
            sscanf(linebuf,"%03d: %03d %03d %03d",&index,&r,&g,&b);
          if(r==-1 || b==-1 || b==-1) PERROR(("Malformed line #%d\n",lc));
          if (program_hardware) program_rgb(index,r,g,b);
        }
      }
      else 
      {
        index = getint(argv[optind], "color index", 0, 255);
        optind++;
        if (argc<optind+1) PERROR(("Missing color value R\n"));
        r = getint(argv[optind], "Color value R", 0, eightbit ? 255 : 63);
        optind++;
        if (argc<optind+1) PERROR(("Missing color value G\n"));
        g = getint(argv[optind], "Color value G", 0, eightbit ? 255 : 63);
        optind++;
        if (argc<optind+1) PERROR(("Missing color value B\n"));
        b = getint(argv[optind], "Color value B", 0, eightbit ? 255 : 63);
        PDEBUG(("Color index = %d (0x%x); Color values : R = %d (0x%x) , G = %d (0x%x) , B = %d (0x%x).\n", index, index, r, r, g, g, b, b));
   
        if (program_hardware) program_rgb(index,r,g,b);
      }
    }
    else
    {  /* getpalette */
      if(strcasecmp(argv[optind],"all"))
      {
        index = getint(argv[optind], "color index", 0, 255);
        if (program_hardware) dumprgb(index, hexdata);
      }
      else
      {
        if (program_hardware) for(index=0;index<256;index++) dumprgb(index, hexdata);
    
      }
    }
  }

#ifndef DOS
  if (program_hardware && setreg && kernel_can_do_cmap && tellkernel)
  {
    if (ioctl(fd, PIO_CMAP, kernel_palette))
    {
       perror("PIO_CMAP");
       PERROR(("Could not do PIO_CMAP on %s\n", ConsoleDevice("0")));
    }
    close (fd);
  }  
#endif
  exit(0);
}

