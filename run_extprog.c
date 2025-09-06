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
 *** run_extprog.c: function to call on external program.
 ***/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "misc.h"
#include "run_extprog.h"
#include "messages.h"

int Run_extern_Prog(char *commandline)
{
  int result=0;
#ifndef RUN_SECURE
  uid_t id = getuid();
	
  (void)setuid(0);
#endif
  PDEBUG(("Executing external command `%s'\n", commandline));
  result=system(commandline);
  if (result !=0)
  {
    perror(commandline);
    PERROR(("'%s' failed with error code %d\n", commandline, result));
  }
#ifndef RUN_SECURE
  setuid(id);
#endif
  return(result);
}

#define FB_FRAGSIZE 1024

static char *extout;

int Run_extern_Prog_pipe(char *commandline)
/* can't use PDEBUG etc. here, because S3 HS text mode would cause garbled output */
{
  int num=0;
  char *extprog;
  FILE *fp_extout;
  
  extprog = strcat(commandline, " 2>&1");
  fp_extout=popen(extprog,"r");
  if (fp_extout==NULL)
  {
    perror(commandline);
    return(-1);
  }

  /* save pipe output to buffer */
  extout = (char *)malloc(FB_FRAGSIZE);
  while ( (extout[num++]=fgetc(fp_extout))!=EOF ) 
  {
    if ( (num % FB_FRAGSIZE) == FB_FRAGSIZE-1)
    {
      extout=realloc((void *)extout, num+1+FB_FRAGSIZE);
    }
  }
  extout[--num]='\0';
  pclose(fp_extout);
  return(0);
}

inline void show_extout()
{
  printf("%s", extout);  
}


