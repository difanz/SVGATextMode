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
 *** tty resizing code for SVGATextMode.
 ***/

#ifndef _TTYRESIZE_H
#define _TTYRESIZE_H

#ifndef DOS
#  include <sys/types.h>

#  include <linux/vt.h> 

#  ifndef NO_RESIZE

     int do_VT_RESIZE(int cols, int rows, int resize1x1);

     int do_VT_RESIZEX(int cols, int rows, int vlin, int clin, int vcol, int ccol, int allow1x1);

     void resize_specified_vts(int cols, int rows);

     void resize_active_vts(int cols, int rows);

#  endif

#else
   int resize_DOS(int cols, int rows, int fontsize); // AB
#endif


int check_if_resize(int cols, int rows);

#endif


