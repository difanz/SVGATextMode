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
 *** Misc. stuff.
 ***/

#ifndef _MISC_H
#define _MISC_H

#include <sys/types.h>

/*
 * Some general purpose stuff
 */


typedef struct {
  const char *name;
  const int token;
} t_name_token;

void *safe_malloc(size_t nbytes);
void *safe_strdup(const char *s);

typedef int bool;

#ifndef TRUE
#  define TRUE (1)
#endif
#ifndef FALSE
#  define FALSE (0)
#endif
#ifndef UNDEFINED
#  define UNDEFINED (-1)
#endif

#ifndef MIN
#  define MIN(a, b)        ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif
 
#endif

