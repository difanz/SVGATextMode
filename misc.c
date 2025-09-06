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
 *** Misc. stuff. & configuration
 ***/

#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "messages.h"
        
void *safe_malloc(size_t nbytes)
{
    void *mem;
    
    if (!(mem = malloc(nbytes)))
    {
      PERROR(("malloc() failed allocating %ud bytes\n", nbytes));
    }
    return mem;
}

void *safe_strdup(const char *s)
{
    char *scopy;
    
    if (!(scopy = strdup(s)))
    {
      PERROR(("strdup() failed allocating memory for a string %ud bytes long\n", strlen(s)));
    }
    return scopy;
}

