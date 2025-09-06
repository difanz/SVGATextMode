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
 *** string_ops.c: string operations
 ***/
 
#include <stdlib.h>
#include "messages.h"

void check_int_range(int cvalue, int lmin, int lmax, char* descstr)
{
  if (cvalue<lmin || cvalue>lmax)
    PERROR(("%s = %d (0x%x) out of range [%d..%d]!\n", descstr, cvalue, cvalue, lmin, lmax));
}

int getint(char* instring, char* descrstring, int lmin, int lmax)
  /* convert the int in 'instring' into an integer. Must be within specified limits 'lmin' and 'lmax' */
  /* 'descrstring' contains a description of the number to be parsed, used in error message */
{
  char** errstr=NULL;
  int readint;
  readint = strtol(instring,errstr,0);
  if (errstr != NULL) PERROR(("Illegal character '%c' in %s: '%s'\n", *errstr, descrstring, instring));
  check_int_range(readint, lmin, lmax, descrstring);
  return(readint);
}

float getfloat(char* instring, char* descrstring, int lmin, int lmax)
  /* convert the float in 'instring' into a float. Must be within specified INTEGER limits 'lmin' and 'lmax' */
  /* 'descrstring' contains a description of the number to be parsed, used in error message */
{
  char** errstr=NULL;
  double readfloat;
  readfloat = strtod(instring,errstr);
  if (errstr != NULL) PERROR(("Illegal character '%c' in %s: '%s'\n", *errstr, descrstring, instring));
  check_int_range(readfloat, lmin, lmax, descrstring);
  return((float)readfloat);
}

