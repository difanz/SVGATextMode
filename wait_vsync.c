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
 *** wait_vsync.c: non-blocking V-blanking wait routine (doesn't hang when no VSYNC)
 ***               waits until start of V-blanking interval.
 ***/

#include <stdio.h>
#ifndef WIN
#  include <values.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "misc.h"
#include "vga_prg.h"
#include "wait_vsync.h"
#include "messages.h"


bool vtimeout=FALSE;   /* will be set TRUE if VSYNC times out */

void nop()
{
}

/* alarm fuction for when probe times out on too slow V-sync */
void badsync(int signal)
{
  PWARNING(("Slow sync. Possibly no, or very slow clock (vertical refresh < 1 Hz).\n"));
  vtimeout=TRUE;
}


int safe_wait_vsync()
{
  /* check if vertical refresh is "reasonable" , and avoid "hangup" when something is really wrong */
  vtimeout=FALSE;
  PDEBUG(("Checking for Slow Sync...\n"));
  signal(SIGALRM, badsync);
  alarm(2);
  wait_vblk;
  alarm(0);
  signal(SIGALRM, SIG_DFL);
  return(!vtimeout);
}
