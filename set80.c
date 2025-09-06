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
 *** set80: sets your VGA card back to 80x25. Useful in case SVGATextMode screws up.
 ***        it will also resize the VT's (when the kernel supports it), but only the first 16 active ones
 ***        Any "special" features that need a TextConfig file are not implemented.
 ***        It is intended to run completely stand-alone.
 ***
 *** Does NOT work for all cards. Those that use EXTENSION bits in addition to the two 
 *** "standard" VGA ones for clock selection, instead if a completely different set of bits
 *** (and then using clock index 2 or 3 to point the VGA chip to the "other" clock selection method)
 *** will probably NOT switch back to the correct clock... 
 *** ATI chips are an example of this.
 ***/

/* #define USE_SVGALIB */

/* if "-a" option defined, use register dump instead of specific register
   programming. A register dump is "safer" because it restores ALL standard
   VGA registers to a known state, so there is more chance of getting a good
   textmode back from it. A specific register programming method changes
   only those registers that need to be changed, and thus less chance of
   changing something that should not be changed...
*/

#ifdef DOS
#  define NO_RESIZE
#endif

#include "misc.h"
#include "ttyresize.h"
#include <unistd.h>
#include <stdio.h>
#ifndef DOS
#  include <sys/ioctl.h>
#  include <sys/kd.h>
#endif
#include "vga_prg.h"
#include "unlock_svga.h"
#include "setclock.h"
#include "std_clock.h"
#include "messages.h"
#include "kversion.h"
#include "file_ops.h"
#include "cfg_data.h"

#ifdef USE_SVGALIB
#include <vga.h>
#endif

#define SET_CLOCKBITS_0_1(no)   ( outb(( inb(VGA_MISC_R) & 0xf3) | (((no) << 2) & 0x0C) , VGA_MISC_W) ) /* bits 0 and 1 of clock no */


/*
 * This mode line will be programmed. It is not used by set80, it's just informational
 */
#define MODELINE "80x25x9" 28.3      640 680 776 800      400 412 414 449   font 9x16
 

/*
 * global variables
 */

char *CommandName;
bool debug_messages=FALSE;

int STM_Options=0;  /* just to keep the compiler happy */

void usage()
{
     PMESSAGE(("version %s. (c) 1995,1996,1997 Koen Gadeyne.\n"\
               "\n"\
               "  Resets and resizes the console to a standard VGA 80x25 text mode\n"\
               "  It is completely stand-alone (does not call other programs, no config file).\n"\
               "  This does _not_ restore the font itself. `setfont' is appropriate for that.\n"\
               "\n"\
               "  Options: -d  print debugging messages\n"\
               "           -h  prints usage information\n"\
               "           -a  use alternate method: restore ALL standard VGA registers.\n"\
               "\n",
     VERSION, CommandName));
}
 

/****************************************************************************************************************************/

int main (int argc, char* argv[])
{
  int c;
  bool restore_all_regs=FALSE;

  CommandName = argv[0];

  while ((c = getopt (argc, argv, "hda")) != EOF)
    switch (c)
    {
      case 'd': debug_messages=TRUE;
                break;
      case 'a': restore_all_regs=TRUE;
                break;
      case 'h': usage();
                exit(0);
                break;
      case '?': usage();
                PERROR(("Bad option '-%c'\n",(char)optopt));
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }

  PVERSION;

#ifndef DOS
#  ifdef NO_RESIZE
    if (check_if_resize(80, 25))
       PWARNING(("Screen resizing was not compiled in (NO_RESIZE defined when compiling).\n\
        Your screen will be garbled, but hopefully useful...\n"));
#  else
    /* we don't check if resizing is necessary: just do it ALWAYS. You never know */
    if (!check_kernel_version(1,1,54, "Virtual Terminal resizing"))
      PWARNING(("Screen resizing not supported by this kernel version.\n\
         Your screen will be garbled, but hopefully useful...\n"));
    else
    {
      do_VT_RESIZE(80, 25, TRUE);  /* always allow going via a 1x1 screen. This is a resque program */
      resize_active_vts(80, 25);
    }
#  endif
#else
   resize_DOS(80, 25, 16);
#endif



#ifndef USE_SVGALIB

  get_VGA_io_perm(CS_VGA);

  outb(0x00, 0x3CC);
  outb(0x01, 0x3CA);

  UNLOCK_STD_VGA;
  
  Set_Textmode(restore_all_regs);

  if (!restore_all_regs)
  {
    Set_MAX_SCANLINE (16);
      
    set_V_timings(400, 412, 414, 449);
    set_H_timings(640, 680, 776, 800);

    Set_CURSOR_START(16-2) ; Set_CURSOR_END(16-1);

    Set_HSYNC_POLARITY(NEG) ; Set_VSYNC_POLARITY(POS);

    set_charwidth(9);
    
    SET_CLOCKBITS_0_1(1);   /* select standard VGA 28 MHZ clock */
    usleep(50000);

    SCREEN_ON;

  }
  
  
#ifndef DOS
  ioctl(opentty( ConsoleDevice("0") ), KDSETMODE, KD_TEXT);
#endif
  
#else  

vga_setmode(TEXT);

#endif
  return(0);

}


