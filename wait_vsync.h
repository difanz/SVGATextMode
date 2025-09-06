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
 *** wait_vsync.h: non-blocking V-sync wait routine (doesn't hang when no VSYNC)
 ***/

#ifndef _WAIT_VSYNC_H
#define _WAIT_VSYNC_H 

#define USE_VSYNC

#ifdef USE_VSYNC
#  define waitframe(todo) \
                 do { \
                   while ( !(inb(STATUS1) & 0x08) && !vtimeout ) todo; /* wait during active display */ \
                   while ( (inb(STATUS1) & 0x08) && !vtimeout ) todo; /* now wait during vert retrace */ \
                 } while (0);

#  define wait_vblk \
                   while ( (inb(STATUS1) & 0x08) && !vtimeout ); /* wait during vert retrace */ \
                   while ( !(inb(STATUS1) & 0x08) && !vtimeout )  /* wait during active display */
#else   /* doesn't work. dunno why */
#  define waitframe(todo) \
                 do { \
                   while ( (inb(STATUS0)) && !vtimeout ) todo; /* wait during active display */ \
                   while ( !(inb(STATUS0)) && !vtimeout ) todo; /* now wait during vert retrace */ \
                 } while (0);

#  define wait_vblk \
                   while ( !(inb(STATUS0)) && !vtimeout ); /* wait during vert retrace */ \
                   while ( (inb(STATUS0)) && !vtimeout )  /* wait during active display */
#endif

extern void nop();
extern bool vtimeout;
int safe_wait_vsync();

#endif


