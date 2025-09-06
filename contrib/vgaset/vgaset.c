/* $Id: vgaset.c,v 1.2 1995/01/23 13:58:59 grog Exp $ 
 *
 * $Log: vgaset.c,v $
 * Revision 1.2  1995/01/23  13:58:59  grog
 * Cosmetic mods
 *
 */
/* vgaset.c: set VGA registers for X server.
 * Greg Lehey */
/* This file is part of vgaset.
 * 
 * vgaset is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * vgaset is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with vgaset; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 *
 * If you modify this software, please let me know how I blew it - I need this information
 * for people who come to me asking about it.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
/* This used to be sys/errno.h, but it broke on some System V machines.
 * This seems to work for all - please contact me if you have trouble */
#include <errno.h>
#include <termios.h>
#ifndef __bsdi__
#include "compiler.h"					    /* this should be the x386/common/compiler.h */
#endif
#ifdef HAS_USL_VTS
#include <sys/kd.h>

#define CONSOLE "/dev/console"				    /* name of console */
int console;						    /* file descriptor for console */
#endif
#include "svga.h"
#ifdef __GNUC__
#define INLINE inline					    /* inline */
#else
#define INLINE						    /* other compilers don't handle it */
#endif

#define MAXEHB	0xf8					    /* highest value of EHB */

struct termios prior_term_status;			    /* terminfos to reset to on close */
struct termios term_status;				    /* and current values */

float dotclock = 0;					    /* dot clock frequency */
char *crtc_reg_name [] =
{
  "HT", "HDE", "SHB", "EHB", "SHR", "EHR", "VT", "OVERFLOW", "PRS",
  "MSL", "CS", "CE", "SAH", "SAL", "CLH", "CLL", "VRS", "EVR", "VDE",
  "OFF", "UL", "VBS", "VBE", "MODE CONTROL", "LC"
    };

/* These are the ``real'' values of the registers. They need to be collected from the
 * VGA registers or scattered to them */
struct crtc_regs
{
  short int hde,					    /* horizontal display end */
	    shr,					    /* start of horizontal retrace */
	    ehr,					    /* end of horizontal retrace */
	    shb,					    /* start of horizontal blanking */
	    ehb,					    /* end of horizontal blanking */
	    ht,						    /* horizontal total */
	    vde,					    /* vertical display end */
	    vbs,					    /* vertical blanking start */
	    vbe,					    /* vertical blanking end */
	    vrs,					    /* vertical retrace start */
	    evr,					    /* end of vertical retrace */
	    vt,						    /* vertical total */
	    overflow,					    /* overflow register */
	    msl;					    /* and maximum scan line reg */
  }
current_regs,
  saved_regs;

int verbose = 0;					    /* set to show more info */
int trident = 0,					    /* set if a TVGA8800 or 8900 */
    tseng = 0;						    /* set if a T3000 or T4000 */

int vgaIOBase;

#ifdef __bsdi__
static INLINE void
  outb (short port, char val)
{
  __asm__ volatile ("out%B0 %0,%1" : :"a" (val), "d" (port));
  }

static INLINE unsigned char
  INB (short port)
{
  unsigned int ret;
  __asm__ volatile ("in%B0 %1,%0" : "=a" (ret) : "d" (port));
  return ret;
  }
#else
static INLINE unsigned char
  INB (short port)
{
  return inb (port) & 0xff;				    /* chop off high-order junk */
  }
#endif
 

/* Exit: clean up and exit */
void Exit (int status)
{
  outb (0x3BF, 0x01);					    /* relock ET4000 special */
  outb (vgaIOBase + 8, 0xA0);
  
#ifdef HAS_USL_VTS
  if (ioctl (console, KDDISABIO, 0) < 0)		    /* disable port I/O access */
    {
    perror ("Can't disable IO ports");
    exit (status + 1);
    }
  if (ioctl (console, KDDELIO, 0x3BF) < 0)		    /* delete port number */
    {
    perror ("Can't delete IO ports");
    exit (status + 1);
    }
#endif
  if (tcsetattr (0, TCSAFLUSH, &prior_term_status))
    {
    printf ("Can't set terminal attributes for stdin: %s\n", strerror (errno));
    Exit (1);
    }
  /* This stuff snarfed from Thomas R|ll's ET4000 driver */
  exit (status);
  }

short int getreg (int address, int index)
{
  int val;
  outb (address, index);				    /* set the register number */
  val = INB (address + 1);
  return val;
  }

void setreg (int address, int index, short int value)
{
  outb (address, index);				    /* set the register number */
  outb (address + 1, value);
  }

void get_crtc_regs ()
{
  current_regs.hde = (getreg (CRTCL_INDEX, HDE) + 1) << 3;
  current_regs.shr = getreg (CRTCL_INDEX, SHR) << 3;
  current_regs.ehr = (getreg (CRTCL_INDEX, EHR) & 0x1f) << 3;
  current_regs.shb = getreg (CRTCL_INDEX, SHB) << 3;
  current_regs.ehb = (getreg (CRTCL_INDEX, EHB) & 0x1f) << 3;
  if (current_regs.ehr <= (current_regs.shr & 0xff))	    /* wraparound, */
    current_regs.ehr += 0x100;				    /* adjust */
  current_regs.ehr += (current_regs.shr & ~0xff);	    /* this is relative to shr */
  current_regs.ht = (getreg (CRTCL_INDEX, HT) + 5) << 3;
  current_regs.overflow = getreg (CRTCL_INDEX, OVERFLOW);
  switch (current_regs.overflow & 0x21)			    /* get the high-order VT stuff */
    {
  case 0:
    current_regs.vt = 0; break;
  case 1:
    current_regs.vt = 256; break;
  case 0x20:
    current_regs.vt = 512; break;
  case 0x21:
    current_regs.vt = 768;
    }
  current_regs.vt += getreg (CRTCL_INDEX, VT) + 2;
  switch (current_regs.overflow & 0x84)			    /* get the high-order VRS stuff */
    {
  case 0:
    current_regs.vrs = 0; break;
  case 4:
    current_regs.vrs = 256; break;
  case 0x80:
    current_regs.vrs = 512; break;
  case 0x84:
    current_regs.vrs = 768;
    }
  current_regs.vrs += getreg (CRTCL_INDEX, VRS);
  if ((current_regs.evr = getreg (CRTCL_INDEX, EVR) & 0xf) <= (current_regs.vrs & 0xf)) /* wraparound */
    current_regs.evr += 0x10;
  current_regs.evr += (current_regs.vrs & ~0xf);	    /* get base */
  switch (current_regs.overflow & 0x42)			    /* get the high-order VDE stuff */
    {
  case 0:
    current_regs.vde = 0; break;
  case 2:
    current_regs.vde = 256; break;
  case 0x40:
    current_regs.vde = 512; break;
  case 0x42:
    current_regs.vde = 768;
    }
  current_regs.vde += getreg (CRTCL_INDEX, VDE) + 1;
  current_regs.vbs = getreg (CRTCL_INDEX, VBS);		    /* start of blanking register */
  if (current_regs.overflow & 8)			    /* bit 8 of VBS reg */
    current_regs.vbs += 256;				    /* add in */
  if (getreg (CRTCL_INDEX, MSL) & 0x20)			    /* bit 9 of VBS reg */
    current_regs.vbs += 512;				    /* add in */
  current_regs.vbe = getreg (CRTCL_INDEX, VBE) & 0x7f;	    /* end of blanking register */
  if (current_regs.vbe < (current_regs.vbs & 0x7f))	    /* wraparound */
    current_regs.vbe += 128;
  current_regs.vbe += current_regs.vbs & ~0x7f;		    /* high-order comes from vbs */
  }

void show_crtc_regs ()
{
  int reg;						    /* register number */
  short int val;					    /* value stored in register */

  get_crtc_regs ();
  if (dotclock)						    /* show frequencies */
    {
    float horfreq = dotclock / current_regs.ht;
    printf ("Horizontal frequency: %5.0f Hz, vertical frequency: %3.1f Hz\n",
	    horfreq,
	    horfreq / current_regs.vt );
    printf ("Horizontal sync %5.2f us, vertical sync %5.2f us\n",
	    (current_regs.ehr - current_regs.shr) / dotclock * 1000000,
	    (current_regs.evr - current_regs.vrs) / horfreq * 1000000 );
    printf ("Horizontal retrace %5.2f us, vertical retrace %5.2f us\n",
	    (current_regs.ht - current_regs.hde) / dotclock * 1000000,
	    (current_regs.vt - current_regs.vde) / horfreq * 1000000 );
    printf ("\"%dx%d\"\t%d\t%d %d %d %d\t%d %d %d %d\n",
	    current_regs.hde,
	    current_regs.vde,
	    (int) dotclock / 1000000,
	    current_regs.hde,
	    current_regs.shr,
	    current_regs.ehr,
	    current_regs.ht,
	    current_regs.vde,
	    current_regs.vrs,
	    current_regs.evr,
	    current_regs.vt);				    /* print clocks line */
    }
  else
    printf ("\"%dx%d\"\t**\t%d %d %d %d\t%d %d %d %d\n",
	    current_regs.hde,
	    current_regs.vde,
	    current_regs.hde,
	    current_regs.shr,
	    current_regs.ehr,
	    current_regs.ht,
	    current_regs.vde,
	    current_regs.vrs,
	    current_regs.evr,
	    current_regs.vt);				    /* print clocks line */
  if (verbose)
    {
    for (reg = 0; reg < CRTC_REG_COUNT; reg++)
      {
      outb (CRTCL_INDEX, reg);				    /* set the register number */
      val = INB (CRTCL_DATA);
      if (! (reg & 3))
	printf ("\n");					    /* 4 to a line */
      printf ("%s\t%x\t", crtc_reg_name [reg], val);
      }
    printf ("\n");
    }
  }

/* set vertical blanking start */
void setvrs ()
{
  current_regs.overflow = getreg (CRTCL_INDEX, OVERFLOW) & ~8;
  if (current_regs.vrs & 0x100)
    current_regs.overflow |= 8;
  setreg (CRTCL_INDEX, OVERFLOW, current_regs.overflow);    /* take care of bit 8 */
  current_regs.msl = getreg (CRTCL_INDEX, MSL) & ~0x20;
  if (current_regs.vrs & 0x200)
    current_regs.msl |= 0x20;
  setreg (CRTCL_INDEX, MSL, current_regs.msl);		    /* take care of bit 9 */
  setreg (CRTCL_INDEX, VRS, current_regs.vrs & 0xff);	    /* and the first 8 bits */
  }

void setevr ()
{
  setreg (CRTCL_INDEX, EVR, (current_regs.evr & 0xf) | (getreg (CRTCL_INDEX, EVR) & ~0xf));
  }

void setvt ()
{
  short int myvt = current_regs.vt - 2;			    /* adjust for VGA format */
  short int highbits [] = {0, 1, 0x20, 0x21};		    /* high-order bits for vt value */
  current_regs.overflow = getreg (CRTCL_INDEX, OVERFLOW) & ~ 0x21;
  current_regs.overflow |= highbits [myvt >> 8];	    /* put in high order bits */
  setreg (CRTCL_INDEX, OVERFLOW, current_regs.overflow);
  setreg (CRTCL_INDEX, VT, myvt & 0xff);		    /* and low-order 8 bits */
  }

void setshr ()
{
  setreg (CRTCL_INDEX, SHR, current_regs.shr >> 3);
  }

void setehr ()
{
  setreg (CRTCL_INDEX, EHR, (getreg (CRTCL_INDEX, EHR) & 0xe0) | ((current_regs.ehr >> 3) & 0x1f));
  }

void setshb ()
{
  setreg (CRTCL_INDEX, SHB, current_regs.shb >> 3);
  }

void setehb ()
{
  setreg (CRTCL_INDEX, EHB, (getreg (CRTCL_INDEX, EHB) & 0xe0) | ((current_regs.ehb >> 3) & 0x1f));
  }

void setvbs ()
{
  setreg (CRTCL_INDEX, OVERFLOW, (getreg (CRTCL_INDEX, OVERFLOW) & ~8) + ((current_regs.vbs >> 4) & 8));
  setreg (CRTCL_INDEX, MSL, (getreg (CRTCL_INDEX, MSL) & ~0x10) + ((current_regs.vbs >> 4) & 0x10));
  setreg (CRTCL_INDEX, VBS, current_regs.vbs);
  }

void setvbe ()
{
  setreg (CRTCL_INDEX, VBE, (getreg (CRTCL_INDEX, VBE) & 0x80) | (current_regs.vbe & 0x3f));
  }

void setht ()
{
  setreg (CRTCL_INDEX, HT, (current_regs.ht >> 3) - 5);
  }

void set_interlace ()
{
  if (tseng)
    setreg (CRTCL_INDEX,
	    TSENG_OVERFLOW_HIGH,
	    (getreg (CRTCL_INDEX, TSENG_OVERFLOW_HIGH) & ~TSENG_INTERLACE) | TSENG_INTERLACE);
  else
    printf ("Sorry, I don't know how to set interlace on this card\n");
  }

void reset_interlace ()
{
  if (tseng)
    setreg (CRTCL_INDEX, TSENG_OVERFLOW_HIGH, getreg (CRTCL_INDEX, TSENG_OVERFLOW_HIGH) & ~TSENG_INTERLACE);
  else
    printf ("Sorry, I don't know how to reset interlace on this card\n");
  }

/* set_register: prompt for a register name and value */
void set_register ()
{
  int i;
  int reg;						    /* register index in crtc register list */
  int found;						    /* flag for register name found */
  char regname [10];
  while (1)						    /* until we hit EOT */
    {
    i = 0;
    printf ("\bSet register: ");
    while (1)
      {
      regname [i] = getchar ();
      if ((regname [i] == prior_term_status.c_cc [VEOF])    /* EOT, */
	  || ((! i) && ((regname [i] == '\r')		    /* CR or */
			|| (regname [i] == '\n') )) )	    /* LF at beginning of line*/
	{
	get_crtc_regs ();				    /* update our memory copy */
	return;
	}
      else if (regname [i] == prior_term_status.c_cc [VERASE]) /* backspace or whatever? */
	{
	if (i > 1)
	  {
	  printf ("\b\b\b");				    /* erase last character on screen */
	  i -= 2;					    /* and in buffer */
	  }
	}
      if ((regname [i] < 'A')				    /* should be end of name */
	  && i)						    /* and we already have something */
	break;
      if (regname [i] >= 'A')				    /* valid character, */
	regname [i++] &= 0x5f;				    /* convert to upper case */
      }
    regname [i] = '\0';					    /* make a string of it */
    puts ("");						    /* new line */
    for (reg = 0; reg < CRTC_REG_COUNT; reg++)
      {
      if (found = (! strcmp (regname, crtc_reg_name [reg]))) /* found the reg, */
	break;
      }
    if (! found)
      printf ("Can't find register %s\n", regname);
    else						    /* got it, show contents */
      {
      char input [20];
      int val;
      outb (CRTCL_INDEX, reg);				    /* set the register number */
      val = INB (CRTCL_DATA);
      printf ("%s: %x -> 0x", regname, val);
      fgets (input, sizeof (input), stdin);		    /* get an input */
      if (sscanf (input, "%x\n", &val))			    /* got a new value */
	{
	outb (CRTCL_INDEX, reg);			    /* set the register number */
	outb (CRTCL_DATA, val);
	outb (CRTCL_INDEX, reg);			    /* set the register number */
	val = INB (CRTCL_DATA);				    /* check it got there */
	printf ("%s: %x\n", crtc_reg_name [reg], val);	    /* and show what we have now */
	}
      }
    }
  }
  
void main (int argc, char *argv [])
{
  int i;
  int temp;
  if (tcgetattr (0, &prior_term_status))		    /* save current terminfos to restore */
    {
    printf ("Can't get terminal attributes for stdin: %s\n", strerror (errno));
    Exit (1);
    }
  if (tcgetattr (0, &term_status))			    /* and get it again to modify */
    {
    printf ("Can't get terminal attributes for stdin: %s\n", strerror (errno));
    Exit (1);
    }
  term_status.c_lflag &= ~ICANON;
#ifdef HAS_USL_VTS					    /* using SVR4 or similar, */
  /* if you set an SVR4 system into raw mode, it will take the VEOF character as the minimum
   * number of characters which will fulfil the read! */
  term_status.c_cc [VMIN] = 1;				    /* complete after one character */
  term_status.c_cc [VTIME] = 0;				    /* and wait forever for a reply */
#endif
  if (tcsetattr (0, TCSAFLUSH, &term_status))
    {
    printf ("Can't set terminal attributes for stdin: %s\n", strerror (errno));
    Exit (1);
    }
  /* This stuff snarfed from Thomas R|ll's ET4000 driver */
#ifdef HAS_USL_VTS
  if ((console = open (CONSOLE, O_RDWR | O_NDELAY)) < 0)    /* can't open console? */
    {
    perror ("Can't open /dev/console");
    exit (1);
    }
  if (ioctl (console, KDADDIO, 0x3BF) < 0)		    /* add port to list */
    {
    perror ("Can't add I/O port 0x3bf");
    exit (1);
    }
  if (ioctl (console, KDENABIO, 0) < 0)
    {
    perror ("Can't enable port I/O");
    exit (1);
    }
#endif
  
  vgaIOBase = (INB (0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
  outb (0x3BF, 0x03);					    /* unlock ET4000 special */
  outb (vgaIOBase + 8, 0xA0);
  outb (CRTCL_INDEX, EVR);				    /* End Vertical Retrace: chop off high-order stuff */
  temp = INB (CRTCL_DATA);
  outb (CRTCL_DATA, temp & 0x7F);
  get_crtc_regs ();					    /* save current values */
  memcpy (&saved_regs, &current_regs, sizeof (struct crtc_regs));
  /* now try to work out what kind of card we have */
  i = getreg (0x3c4, 0);				    /* get mode control #1 */
  setreg (0x3c4, 0, 0);
  if (getreg (0x3c4, 0) == 2)				    /* inverted bit 1 */
    trident = 1;
  setreg (0x3c4, 0, i);					    /* restore old contents */
  for (i = 1; i < argc; i++)
    {
    if (*argv [i] == '-')				    /* flag */
      {
      switch (argv [i] [1])
	{
      case 'd':
	if ((dotclock = atof (&argv [i] [2])) < 200)	    /* get it */
	  dotclock *= 1000000;				    /* convert to MHz */
	break;
      case 'i':
	reset_interlace ();
	break;
      case 'I':
	set_interlace ();
	break;
      case 's':						    /* set the registers first */
	if (argc < (i + 7))				    /* not enough args */
	  {
	  puts ("Not enough arguments to -s option");
	  Exit (1);
	  }
	current_regs.shr = atoi (argv [++i]);		    /* get the values to set the regs to */
	current_regs.ehr = atoi (argv [++i]);
	current_regs.ht = atoi (argv [++i]);
	setshr ();
	setehr ();
	setht ();
	current_regs.vrs = atoi (argv [++i]);
	current_regs.evr = atoi (argv [++i]);
	current_regs.vt = atoi (argv [++i]);
	setvrs ();
	setevr ();
	setvt ();
	break;
      case 'v':
	verbose = 1;
	break;
	}
      }
    else
      {
      printf ("Usage:\n%s [-ddot clock] [-s shr ehr ht vrs evr vt] [-v]\n", argv [0]);
      Exit (1);
      }
    }
  if (trident)
    {
    if (verbose)
      printf ("Looks like a Trident TVGA\n");
    }
  else							    /* see if it's a Tseng */
    {
    /* Tseng identification sequence as per Ferraro, "Programmer's guide to the EGA and VGA cards",
     * page 936. The only problem with this code is that it does not work (just blanks the screen). */
#ifdef FUNCTIONAL
    outb (0x3bf, 3);
    if (INB (0x3cc) == 0xd1)
      outb (0x3d8, 0xa0);
    else
      outb (0x3b8, 0xa0);
    INB (0x3da);					    /*	set attribute index */
    outb (0x3c0, 0x16);
    i = INB (0x3c1);					    /* get ATC misc register */
    INB (0x3da);
    outb (0x3c0, 0x16);
    outb (0x3c0, i ^ 0x10);				    /* set ATC misc register */
    INB (0x3da);
    outb (0x3c0, 0x16);
    tseng = INB (0x3c1) == (i ^ 0x10);			    /* if it stayed that way, it's a Tseng */
    INB (0x3da);
    outb (0x3c0, 0x16);
    outb (0x3c0, i);
#else							    /* fudge it */
    tseng = 1;
#endif
    if (tseng && verbose)
      puts ("Looks like a Tseng chip");	     
    }
  show_crtc_regs ();					    /* first time round */
  while (1)
    {
    char c = getchar ();
    if (c == prior_term_status.c_cc [VEOF])		    /* EOT char, */
      Exit (0);						    /* stop */
    switch (c)
      {
    case 'i':
      puts ("nterlace off");
      reset_interlace ();
      break;
    case 'I':
      puts ("nterlace on");
      set_interlace ();
      break;
    case 'l':						    /* decrease left */
      puts ("eft margin decrease");
      if (current_regs.ht > current_regs.ehr)
	{
	current_regs.ht -= 8;
	setht ();
	}
      else
	printf ("Can't reduce left margin further\n\007");
      break;
    case 'L':						    /* increase left */
      puts ("eft margin increase");
      if (current_regs.ht < 2048)			    /* space to increase */
	{
	current_regs.ht += 8;
	setht ();
	}
      else
	printf ("Can't increase horizontal total further\007\n");
      break;
    case 'r':						    /* decrease right */
      puts ("ight margin decrease");
      if (current_regs.shr > current_regs.hde)		    /* can decrease */
	{
	current_regs.shr -= 8;
	current_regs.ehr -= 8;
	current_regs.ht -= 8;
	setshr ();
	setehr ();
	setht ();
	/* blanking is not configured explicitly, but if we don't track our sync we're liable
	 * to end up blanking parts of the image */
	current_regs.shb -= 8;
	if (current_regs.ehb)				    /* ehb < 0? */
	  current_regs.ehb -= 8;
	setshb ();
	setehb ();
	}
      else
	printf ("Can't decrease right margin further\007\n");
      break;
    case 'R':						    /* increase right */
      puts ("ight margin increase");
      if (current_regs.ht < 2048)			    /* space to increase */
	{
	current_regs.ht += 8;
	current_regs.ehr += 8;
	current_regs.shr += 8;
	setshr ();
	setehr ();
	setht ();
	current_regs.shb += 8;
	if (current_regs.ehb > MAXEHB)			    /* ehb < max? */
	  current_regs.ehb += 8;
	setshb ();
	setehb ();
	}
      else
	printf ("Can't increase right margin further\007\n");
      break;
    case 'h':						    /* decrease horizontal sync */
      puts ("orizontal sync decrease");
      if (current_regs.ehr > (current_regs.shr + 8))	    /* can decrease */
	{
	current_regs.ehr -= 8;
	setehr ();
	if (current_regs.ehb)				    /* ehb < 0? */
	  current_regs.ehb -= 8;
	setehb ();
	}
      else
	printf ("Can't decrease horizontal sync further\007\n");
      break;
    case 'H':						    /* increase horizontal sync */
      puts ("orizontal sync increase");
      if (current_regs.ehr < current_regs.ht)		    /* space to increase */
	{
	current_regs.ehr += 8;
	setehr ();
	if (current_regs.ehb > MAXEHB)			    /* ehb < max? */
	  current_regs.ehb += 8;
	setehb ();
	}
      else
	printf ("Can't increase horizontal sync further\007\n");
      break;
/* Vertical operations */
    case 't':						    /* decrease top */
      puts ("op margin decrease");
      if (current_regs.vt > current_regs.evr)
	{
	current_regs.vt -= 1;
	setvt ();
	}
      else
	printf ("Can't reduce top margin further\n\007");
      break;
    case 'T':						    /* increase top */
      puts ("op margin increase");
      if (current_regs.vt < 1024)			    /* space to increase */
	{
	current_regs.vt += 1;
	setvt ();
	}
      else
	printf ("Can't increase vertical total further\007\n");
      break;
    case 'b':						    /* decrease bottom */
      puts ("ottom margin decrease");
      if (current_regs.vrs > current_regs.vde)		    /* can decrease */
	{
	current_regs.vrs -= 1;
	current_regs.evr -= 1;
	current_regs.vt -= 1;
	setvrs ();
	setevr ();
	setvt ();
	current_regs.vbs -= 1;
	current_regs.vbe -= 1;
	setvbs ();
	setvbe ();
	}
      else
	printf ("Can't decrease bottom margin further\007\n");
      break;
    case 'B':						    /* increase bottom */
      puts ("ottom margin increase");
      if (current_regs.vt < 1024)			    /* space to increase */
	{
	current_regs.vt += 1;
	current_regs.evr += 1;
	current_regs.vrs += 1;
	setvrs ();
	setevr ();
	setvt ();
	current_regs.vbs += 1;
	current_regs.vbe += 1;
	setvbs ();
	setvbe ();
	}
      else
	printf ("Can't increase bottom margin further\007\n");
      break;
    case 'v':						    /* decrease vertical sync */
      puts ("ertical sync decrease");
      if (current_regs.evr > current_regs.vrs)		    /* can decrease */
	{
	current_regs.evr -= 1;
	setevr ();
	current_regs.vbe -= 1;
	setvbe ();
	}
      else
	printf ("Can't decrease vertical sync further\007\n");
      break;
    case 'V':						    /* increase vertical sync */
      puts ("ertical sync increase");
      if (current_regs.evr < current_regs.vt)		    /* space to increase */
	{
	current_regs.evr += 1;
	setevr ();
	current_regs.vbe += 1;
	setvbe ();
	}
      else
	printf ("Can't increase vertical sync further\007\n");
      break;
    case '\n':						    /* do nothing */
      break;
    case '?':						    /* reset saved values */
      puts ("\bReset to initial values");
      memcpy (&current_regs, &saved_regs, sizeof (struct crtc_regs));
      setvrs ();
      setevr ();
      setvt ();
      setshr ();
      setehr ();
      setht ();
      setshb ();
      setehb ();
      setvbs ();
      setvbe ();
      break;
    case '=':						    /* set register explicitly */
      set_register ();
      break;
    case 'q':
      puts ("uit");
      Exit (0);
      /* NOTREACHED */
    default:
      printf ("Invalid command\007\n");
      }
    show_crtc_regs ();
    }
  }
