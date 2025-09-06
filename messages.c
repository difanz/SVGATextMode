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
 *** message printing tools
 ***/

#include <stdio.h>
#include <stdarg.h>
#include "misc.h"
#include "messages.h"

int msgtype;  
  
void print_msg(char *format,...)
{
  char *typstr[4] = { "DEBUG: " , "" , "WARNING: " , "ERROR: " };
  va_list argptr;
  FILE *stream;

  if ((msgtype==MSGTYP_DBG) && (debug_messages==FALSE)) return;
  if (msgtype==MSGTYP_MSG) stream = stdout;
    else stream = stderr;
  va_start(argptr,format);
  if (msgtype==MSGTYP_DBG)
    fprintf(stream,"%s", typstr[msgtype]);
  else    fprintf(stream,"%s: %s", CommandName, typstr[msgtype]);
  vfprintf(stream,format,argptr);
  va_end(argptr);
  fflush(stream);
}


