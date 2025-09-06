/*
Unlike "vcs", reading "vcsa" from offset 0 returns a buffer with a 4-byte
header containing (as I recall) rows, cols, cur_col, cur_row, in that order.
When placing new data in the buffer, offset 4 should be treated as 0, but
the whole buffer, header included, should be written back from 0.
*/
/*
Here's my "port" of Borland C's gettext() and puttext(), using /dev/vcsa0.
They read/write rectangular areas of the screen.

written by Bob McCracken
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEV	"/dev/vcsa0"

#ifndef uchar
#define uchar unsigned char
#endif

int GetText (uchar *buf, int y1, int x1, int y2, int x2)
{
  int Bpr, p, h, v, x, y;
  uchar *M, *Mem;
  struct winsize W;

  ioctl (0, TIOCGWINSZ, &W);

/*
Borland's version defaults to the real full-screen size if the
parms are invalid. Not liking that, I return an error, instead.
*/  
  if (x1 < 0 || y1 < 0 || x2 >= W.ws_col || y2 >= W.ws_row ||

	x1 > x2 || y1 > y2) return -1;
	  
  if ((y = open (DEV, 0)) < 0) return -1;
  
  Bpr = (W.ws_col << 1);	/* bytes-per-row */
  
  x = (Bpr * W.ws_row) + 4;	/* Include 4-byte header in buffer size */
  
  Mem = (uchar *) malloc (x);	/* Ought to check for failure here! */
  
  M = Mem + 4;		/* Point M to start of actual screen data */
  
  read (y, Mem, x); close (y);

/*
"Rectangularize" the screen data and copy to caller's buffer ..
*/  
  v = y2-y1+1; h = (x2-x1+1) << 1; p = (y1 * Bpr) + (x1 << 1);

  for (x = y = 0; y < v; y++, p += Bpr, x += h) memcpy (buf+x, M+p, h);

  free (Mem); return 0;
}

/* Same as above, in reverse ... */

int PutText (uchar *buf, int y1, int x1, int y2, int x2)
{
  int fd, ss, Bpr, p, h, v, x, y;
  uchar *M, *Mem;
  struct winsize W;

  ioctl (0, TIOCGWINSZ, &W);
  
  if (x1 < 0 || y1 < 0 || x2 >= W.ws_col || y2 >= W.ws_row ||
  
  	x1 > x2 || y1 > y2) return -1;
  	
  if ((fd = open (DEV, 2)) < 0) return -1;
  
  Bpr = (W.ws_col << 1);
  
  ss = (Bpr * W.ws_row) + 4;
  
  Mem = (uchar *) malloc (ss);
  
  M = Mem + 4;

/* Get current screen contents and update with caller's "rectangle" */

  read (fd, Mem, ss); lseek (fd, 0, 0);
  
  v = y2-y1+1; h = (x2-x1+1) << 1; p = (y1 * Bpr) + (x1 << 1);

  for (x = y = 0; y < v; y++, p += Bpr, x += h) memcpy (M+p, buf+x, h);

  write (fd, Mem, ss); close (fd);
  
  free (Mem); return 0;
}

/* Usage example: Change background color of a small area. */

#ifdef TEST_GETPUT

#define BGND	0x4F	/* white on red */

#define Y1	6
#define	X1	10
#define	Y2	12
#define X2	39

#define XLEN	(X2-X1+1)
#define YLEN	(Y2-Y1+1)

#define BSIZE	((XLEN<<1)*YLEN)

uchar scrn [BSIZE];

void main()
{
  int x;
  
  if (GetText (scrn, Y1,X1, Y2,X2) < 0) printf ("oops"); else
    {  
	for (x = 1; x < BSIZE; x += 2) scrn[x] = BGND;

	PutText (scrn, Y1,X1, Y2,X2);
    }	
}
#endif
