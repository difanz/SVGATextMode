/*  SVGATextMode -- An SVGA textmode manipulation/enhancement tool
 *
 *  Copyright (C) 1995,1996  Koen Gadeyne
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
 *** mach64_mem_access.c, grants memory access. Mostly used
 *** for fetching BIOS data.
 *** Contributed by M. Grimrath (m.grimrath@tu-bs.de)
 ***/



/************************************************************************/
/************************* Linux ****************************************/
/************************************************************************/

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "messages.h"

#if defined(linux)
/************************************************************************/
int memacc_open(void)
{ int mem;
  
  if ((mem = open("/dev/mem",O_RDONLY)) < 0) {
    perror("/dev/mem");
  }
  return mem;
}



/************************************************************************/
void memacc_fetch(int handle, unsigned long addr, void *dst, unsigned len)
{ lseek(handle, addr, SEEK_SET);
  read(handle, dst, len);
}

/************************************************************************/
void memacc_close(int handle)
{ if (handle < 0) {
    PERROR((__FILE__": internal: illegal handle given!\n"));
  }
  close(handle);
}



/************************************************************************/
/************************** DOS *****************************************/
/************************************************************************/
#elif defined(DJGPP)
/* UNTESTED!!! */

  /* Really not possible in DOS? The mmap-stuff can be ported to DOS like in mga_clock.c
   * Just need someone to test it. - Wolfram, Jan 06
   */
int memacc_open(void)
{ PERROR(("Sorry, MACH64 not (yet) supported in DOS! See INSTALL.TXT.\n"));
}

void memacc_fetch(int handle, unsigned long addr, void *dst, unsigned len)
{ PERROR(("Sorry, MACH64 not (yet) supported in DOS! See INSTALL.TXT.\n"));
}

void memacc_close(int handle)
{ PERROR(("Sorry, MACH64 not (yet) supported in DOS! See INSTALL.TXT.\n"));
}



/************************************************************************/
#else
#error Unsupported system/compiler!
#endif
