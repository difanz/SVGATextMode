/*
The code below is a simple test/demo I wrote, to be adapted later
into a replacement for my current color-setting program. It speaks
for itself.

I run it from rc.local at startup, thus:

	/usr/local/sbin/setcolor 1 112 0 0 4 0 0 112 8 0 0 0

It works, and the colors are "sticky".

If you find any of this code helpful, feel free to use it.


- Bob McCracken		kerouac@ssnet.com
*/

/*
	The following info was obtained by examining the file
	
		/usr/src/linux/drivers/char/vga.c

	in the source for kernel 1.3.47.
	
1. Each palette reg points to the dac reg of the same number:

	palette[0] = 0
	...
	...
	palette[15] = 15
		
   That is, the DOS method of setting palette regs 8 - 15 to
   dac regs 56 - 63 (and 6 to 20) is no longer implemented.

2. The palette numbering scheme conforms to the ANSI standard,
   (1 = red, 4 = blue, etc.), NOT to the VGA numbering scheme
   (1 = blue, 4 = red, etc.).
	
3. RGB levels are in the range 0 - 255, not 0 - 63.

4. RGB levels are set en bloc, not by individual dac regs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

typedef struct { unsigned char red, green, blue; } dacreg;

typedef struct { dacreg d [16]; } ColorMap;

/*
Note on the above:

A simple array ( "unsigned char map[48]" ) would work just as well as a
structure, but would require a set of #defines to make it easier to
reference the r-g-b parts of the dac regs. (Note that if an array is
used instead, "&map" must still be used in the ioctl(), since the latter
assumes it's being passed a structure address.)
*/

/*
My kernel's defaults. These were obtained with
"setcolor -d" after booting, and copied in here:
*/
dacreg CurrentColors[16] = {

	{0x00, 0x00, 0x00}, {0xAA, 0x00, 0x00},
	{0x00, 0xAA, 0x00}, {0xAA, 0x55, 0x00},
	{0x00, 0x00, 0xAA}, {0xAA, 0x00, 0xAA},
	{0x00, 0xAA, 0xAA}, {0xAA, 0xAA, 0xAA},
	{0x55, 0x55, 0x55}, {0xFF, 0x55, 0x55},
	{0x55, 0xFF, 0x55}, {0xFF, 0xFF, 0x55},
	{0x55, 0x55, 0xFF}, {0xFF, 0x55, 0xFF},
	{0x55, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}	};

ColorMap map;

void reset()
{
  int x;

  for (x = 0; x < 16; x++)
     {
  	map.d[x].red	= CurrentColors[x].red;
  	map.d[x].green	= CurrentColors[x].green;
  	map.d[x].blue	= CurrentColors[x].blue;
     }

  ioctl (0, PIO_CMAP, &map);
}

void display()
{
  int x;

  ioctl (0, GIO_CMAP, &map);

  printf ("\ndacreg CurrentColors[16] = {\n\n");
  
  for (x = 0; x < 16; x++)

    printf ("\t{0x%02X, 0x%02X, 0x%02X}\t/* %2d */\n",
	map.d[x].red, map.d[x].green, map.d[x].blue, x);
	
  printf ("};\n\n");
}

void main (int argc, char **argv)
{
  extern int opterr, optind;

  int x, Reset, Display, r, g, b, reg;

  if (argc < 2) Display = 1; else	/* do "-d" if no args */
    {
      opterr = Reset = Display = 0;

      while ((x = getopt (argc, argv, "dr")) != EOF) switch (x)
	{
	  case 'd' : Display	= 1;	break;
	  case 'r' : Reset	= 1;	break;
	  default  :			break;
	}

      if (Reset) reset(); else if (argv [optind])
	{
	  if ((argc - optind) % 4)	/* valid numeric arg count? */
	    {
	      printf ("usage: %s [-d] [-r] [reg r g b [reg r g b ..]]\n",
			argv[0]);
              return;
            }
            
	  ioctl (0, GIO_CMAP, &map);

	  for (x = optind; x < argc; x += 4)
	     {
		if ((reg = atoi (argv[x])) >= 0 && reg < 16)
		  {
			r = atoi (argv[x+1]);
			g = atoi (argv[x+2]);
			b = atoi (argv[x+3]);

			if (r >= 0 && r < 256 &&
			    g >= 0 && g < 256 &&
			    b >= 0 && b < 256)
			  {
				map.d[reg].red	= r;
				map.d[reg].green= g;
				map.d[reg].blue	= b;
			  }
		  }
	     }

	  ioctl (0, PIO_CMAP, &map);
	}
    }

  if (Display) display();
}

