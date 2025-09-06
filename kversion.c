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
 *** Kernel version checking code
 ***/

#include <stdio.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include "messages.h"
#include "kversion.h"

int check_kernel_version(int req_ver, int req_rel, int req_pl, char* reason)
{
  struct utsname my_utsname;
  int kversion, kv_ver, kv_rel, kv_pl;
  int req_version = req_ver*1000000+req_rel*1000+req_pl;

  if (uname(&my_utsname))
  {
    perror("uname");
    PWARNING(("Could not get kernel version number. Assuming worst case (<1.1.54) ...\n"));
    return(FALSE);
  }
  
  PDEBUG(("Checking if kernel supports %s (required: %d.%d.%d ; this: %s)\n",
           reason, req_ver, req_rel, req_pl, my_utsname.release)); 
  sscanf(my_utsname.release, "%d.%d.%d", &kv_ver, &kv_rel, &kv_pl);
  kversion = kv_ver*1000000+kv_rel*1000+kv_pl;
  return(kversion >= req_version);
}

