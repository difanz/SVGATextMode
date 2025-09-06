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
 *** grabmode/clockprobe
 ***
 *** This program outputs the current text or graphics mode, in an SVGATextMode (or XF86Config-like) compatible line
 *** Should work on ANY VGA card, since it only uses standard VGA registers
 ***
 ***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "misc.h"
#include "messages.h"
#include "modedata.h"
#include "mode_msg.h"
#include "string_ops.h"
#include "cfg_data.h"

#ifdef DOS
#define get_VGA_io_perm(x)
#undef Renounce_SUID
#define Renounce_SUID
extern int optind;
#endif

#ifdef win
#  include "asm/io.h"
#endif

char *CommandName;
bool debug_messages=FALSE;

int STM_Options=0;  /* just to keep the compiler happy */

#define CLOCKPROBE  1
#define GRABMODE    2

void usage(int func)
{
     PMESSAGE(("version %s. (c) 1995-1998 Koen Gadeyne.\n Usage: %s [options] \n"\
     "  Options: -n  Don't program VGA hardware\n"\
     "           -d  print debugging information\n"\
     "           -r  `raw' mode: don't do any guesswork on special modes,\n"\
     "               just print the values as they appear in the VGA registers\n"\
     "           -v  verbose mode (report all special VGA flags for the mode)\n"\
     "           -x <screen width>\n"\
     "           -y <screen height>\n"\
     "              specify screen width and height in pixels when it is known\n"\
     "              beforehand. This reduces the probe's guesswork (and errors).\n"\
     "           -h  print usage information\n"\
     "%s"\
     "  NOTE: All non-standard VGA features could cause bogus results:\n"\
     "         * interlaced modes\n"\
     "         * modes with >= 1024 active lines (like 1280x1024 and up)\n"\
     "         * HiColor (15/16bpp) and TrueColor (24/32bpp) modes\n"\
     "         * modes on cards with special DAC's\n"\
     "        The program will try to detect this and compensate\n"\
     "        (but will issue a warning that the actual VGA timings were adjusted)\n",\
     VERSION, CommandName,\
     (func==GRABMODE) ? \
     "           -X  Output compatible with XF86Config mode line\n"\
     "               (NOTE: grabbed graphic modes are not always correct)\n"\
     "           -T  Output compatible with TextConfig mode line\n"\
     "           -b  Also print blanking information\n"\
     "           -c  Don't probe for pixel clock (just print timing information)\n"\
     :\
     "           -p  print ONLY clock value (for use in scripts, pipes, etc)\n" ));
}

/***********************************************************************************************************/
 
int main (int argc, char* argv[])
{
  bool program_hardware=TRUE;
  int Xmode=MODE_UNDEFINED;
  bool showblank=FALSE;
  bool probe_clock=TRUE;
  bool pipeable=FALSE;
  bool raw_mode=FALSE;
  bool verbose=FALSE;
  int i;
  char* commandfilename;
  int c;
  modestruct m;
  int initial_x=-1, initial_y=-1;
  int func;
  
  CommandName = argv[0];
  commandfilename = strrchr(CommandName, '/');
  if (commandfilename) commandfilename++;
  else commandfilename = CommandName;

#ifdef DOS
  func = (!strncmp(commandfilename,"grab",4)) ? GRABMODE : CLOCKPROBE;
#else
  func = (!strncasecmp(commandfilename,"grab",4)) ? GRABMODE : CLOCKPROBE;
#endif

 
 /*
  * command-line argument parsing
  */

#ifdef WIN
   verbose=TRUE;
   debug_messages=TRUE;
#else
  while ((c = getopt (argc, argv, "ndrhXTbcpvx:y:")) != EOF)
    switch (c)
    {
      case 'n': program_hardware=FALSE;
                break;
      case 'd': debug_messages=TRUE;
                break;
      case 'r': raw_mode=TRUE;
                break;
      case 'X': Xmode = MODE_GRAPHICS;
                break;
      case 'T': Xmode = MODE_TEXT;
                break;
      case 'b': showblank = TRUE;
                break;
      case 'c': probe_clock = FALSE;
                break;
      case 'p': pipeable = TRUE;
                break;
      case 'v': verbose = TRUE;
                break;
      case 'h': usage(func);
                exit(0);
                break;
      case 'x': initial_x=getint(optarg, "X-size preset", 0, 4096);
                PDEBUG(("X-size on command line = %d\n", initial_x));
                break;
      case 'y': initial_y=getint(optarg, "Y-size preset", 0, 4096);
                PDEBUG(("Y-size on command line = %d\n", initial_y));
                break;
      case '?': usage(func);
#ifndef DOS
                PERROR(("Bad option '-%c'\n",(char)optopt));
#endif                
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }
#endif

 PVERSION;

/*
 * start doing something useful
 */
 
 if (!program_hardware) return(0);

 PWARNING(("Please be patient. This may take a while (up to 1 minute)\n"));

 getmode(&m, probe_clock, raw_mode, initial_x, initial_y);

 if (func == CLOCKPROBE)
 {
   if (pipeable)
   {
     printf("%1.2f\n", m.mode_line.pixelClock/1000.0);
   }
   else
   {
     PMESSAGE(("Estimated vertical scanrate = %1.3f Hz.\n", m.mode_line.vfreq/1000.0));
     PMESSAGE(("Estimated horizontal scanrate = %1.3f kHz.\n", m.mode_line.hfreq/1000.0));
     PMESSAGE(("Estimated pixel clock = %1.2f MHz\n", m.mode_line.pixelClock/1000.0));
     if (GMOFLG_ISSET(m,CLOCKDIV2))
       PMESSAGE(("NOTE: actual clock is %1.2f MHz, but it is divided by 2.\n", m.mode_line.pixelClock*2/1000.0));
   }
   return(0);
 }

 /*
  * "grabmode" function.
  * From parameters, look for special settings that affect the output
  */
  
  /* auto selection between graphics/text mode if not defined on command line */
  if (Xmode==MODE_UNDEFINED) Xmode = m.txt_gr_mode;
  
 /*
  * Print the mode line
  */
  
 if (Xmode==MODE_TEXT)
   printf("\"%dx%dx%d\"   ", m.mode_line.HDisplay/8, m.mode_line.VDisplay/m.mode_line.FontHeight, m.mode_line.FontWidth);
 else 
   printf("\"%dx%d\"   ", m.mode_line.HDisplay, m.mode_line.VDisplay);

 if (probe_clock)
   printf("%1.3f", m.mode_line.pixelClock/1000.0);
 else
   printf("***");
         
 printf("   %d %d %d %d   %d %d %d %d   %cHsync %cVsync",
         m.mode_line.HDisplay, m.mode_line.HSyncStart, m.mode_line.HSyncEnd, m.mode_line.HTotal,
         m.mode_line.VDisplay, m.mode_line.VSyncStart, m.mode_line.VSyncEnd, m.mode_line.VTotal,
        (m.mode_line.hpol==POS) ? '+' : '-',
        (m.mode_line.vpol==POS) ? '+' : '-');
        
 if (GMOFLG_ISSET(m,DOUBLESCAN)) printf("  DoubleScan");
 if (GMOFLG_ISSET(m,MULTISCAN)) printf("  %sScan", m.mode_line.FontHeight==2 ? "Double" : "Multi");

 if (GMOFLG_ISSET(m,INTERLACE)) printf("  Interlace");

 if (m.mode_line.hshift) printf("  hshift %d", m.mode_line.hshift);

 if (Xmode==MODE_TEXT) printf("  font %dx%d", m.mode_line.FontWidth, m.mode_line.FontHeight);
 
 if (probe_clock)    
   printf("   # %1.3fkHz/%1.2fHz\n", m.mode_line.hfreq/1000.0, m.mode_line.vfreq/1000.0);
 else
   printf("\n");
   
 if (showblank)
   printf("#   Blanking: H=%d-%d ; V=%d-%d\n", m.starthbl, m.endhbl, m.startvbl, m.endvbl);
   
 if (verbose)
 {  
   printf("#   Active special VGA register flags: ");
   if (m.mode_flags == 0) printf("none.");
   if (GMOFLG_ISSET(m,DOUBLESCAN))      printf("DOUBLESCAN ");
   if (GMOFLG_ISSET(m,CLOCKDIV2))       printf("CLOCKDIV2 ");
   if (GMOFLG_ISSET(m,MULTISCAN))       printf("MULTISCAN ");
   if (GMOFLG_ISSET(m,BYTEMODE))        printf("BYTEMODE ");
   if (GMOFLG_ISSET(m,WORDMODE))        printf("WORDMODE ");
   if (GMOFLG_ISSET(m,DOUBLEWORDMODE))  printf("DOUBLEWORDMODE ");
   if (GMOFLG_ISSET(m,VERT_DOUBLE))     printf("VERT_DOUBLE ");
   if (GMOFLG_ISSET(m,PCS_DIV2))        printf("PCS_DIV2 ");
   if (GMOFLG_ISSET(m,CNT_BY_2))        printf("CNT_BY_2 ");
   if (GMOFLG_ISSET(m,INTERLACE))       printf("INTERLACE ");
   if (GMOFLG_ISSET(m,SHIFT_CGA256C))   printf("SHIFT_CGA256C ");
   if (GMOFLG_ISSET(m,SHIFT_CGA4C))     printf("SHIFT_CGA4C ");
   printf("\n");
 }  
 
 if (m.remarks!=0)
 {
   PDEBUG(("Remarks mask: 0x%4X\n", m.remarks));
   fputc('\n', stderr);
   PWARNING(("Grabbed mode could be wrong. Below is a list of remarks about it:\n\n"));
   for (i=0; mode_messages[i].msg_mask >0 ; i++)
   {
     if ((m.remarks & (1<<i)) != 0)
     {
       if ((1<<i)==MSG_CLOCK_MEASUREMENTS)
         fprintf(stderr, mode_messages[i].msg_txt,m.valid_measurements);
       else
         fprintf(stderr, mode_messages[i].msg_txt);
       fputc('\n', stderr);
     }
   }
   return(1);
 }
  return(0);
}
