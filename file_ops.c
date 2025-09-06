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
 *** file_ops.c: file operation functions
 ***/
 
#include <stdio.h>
#include <fcntl.h>   /* for open() */
#include "messages.h"

/* With DOS, the configfile is now searched in the current directory or
 * along the "PATH"-variable. This is more DOS-stlye than "/etc/...".
 * - Wolfram, Jan 06
 */
#ifdef DOS
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *path_token(const char *s, char *d, size_t maxlen)
{
  if (!*s) return NULL;

  while ( *s && d < d+maxlen ) {
    if ( *s == ';' ) {
      while ( *(++s) == ';') ;
      break;
    }
    *d++ = *s++;
  }
  if (*(d-1) != '\\') *d++ = '\\';
  *d = 0;
  return (char *)s;
}


FILE *path_fopen(char *name, char *mode)
{
      char *path;
      char filename[FILENAME_MAX];

      if (access(name, F_OK) == 0)
        return(fopen(name, mode));

      if (!(path = getenv("PATH")))
        return NULL;

      while ((path = path_token(path, filename, FILENAME_MAX - strlen(name) - 2))) {
         strcat(filename, name);
         if (access(filename, F_OK) == 0)
           return (fopen(filename, mode));
      }
      return NULL;
}

#endif /* DOS */


FILE* open_param_file(char* conf_file)
{
  FILE* param_file;
  PDEBUG(("Opening config file '%s'\n",conf_file));
#ifndef DOS
  if ((param_file = fopen(conf_file,"r")) == NULL)
#else
  if ((param_file = path_fopen(conf_file,"r")) == NULL)
#endif
  {
      perror("fopen");
      PERROR(("Could not open Text mode config file '%s'\n",conf_file));
  }
  return(param_file);
}

int opentty(const char *devname)
{
  int fd;

  fd = open(devname, O_WRONLY | O_NOCTTY);
  if (fd < 0)
  {
     perror("open");
     PERROR(("Could not open %s for writing\n", devname));
     return -1;
  }
  return(fd);
}

