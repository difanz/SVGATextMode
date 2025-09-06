/*
 * Screen-capture program for Linux text consoles

 * The "vcs" and "vcsa" drivers give read/write access to a vt's video memory,
 * somewhat like direct access to segment 0xb800 under DOS. The vcs version is
 * character-only, the vcsa version is character-attribute. There's one for
 * each vt, and they're numbered 7,0+ and 7,128+, respectively, with
 * /dev/vcs[a]0 corresponding to "/dev/tty". 
 */

/*
	dscrn.c   (c) Bob McCracken

Usage: dscrn [ttyno] > foo
*/

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/major.h>

#define PSIZE	4096		/* screen page size */
#define BSIZE	(PSIZE << 3)	/* 8 pages per vt */

char buf [BSIZE];

void main (int argc, char **argv)
{
  int c, x, y; char dev [16]; struct winsize w;
  
  x = (argc < 2) ? 0 : atoi(argv[1]);

  ioctl (0, TIOCGWINSZ, &w);
  
  if (x >= 0 && x <= MAX_CHRDEV)	/* ttyno in range? */
    {           
      sprintf (dev, "/dev/vcs%d", x);	/* only want chars, not attrs */

      if ((x = open (dev, 0)) >= 0)
	{ 
	  y = read (x, buf, BSIZE); close (x);
	  
	  for (c = x = 0; x < y; x++)
	     {
	       if (putchar (buf[x]) == '\n') c = 0;	/* (Actually, there
							shouldn't be any
							newlines in it.) */

		else if (++c >= w.ws_col) { c = 0; putchar ('\n'); }
             }
	}
    }
}
