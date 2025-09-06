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
 *** main SVGA clock programming functions for SVGATextMode
 ***/

#ifndef _SETCLOCK_H
#define _SETCLOCK_H

/*
 * Clock selection error codes (MUST be negative !!!)
 */

#define CLKSEL_OUT_OF_BOUNDS    -4
#define CLKSEL_DONOTHING        -3
#define CLKSEL_ILLEGAL_NUM      -2
#define CLKSEL_NULLCLOCK        -1


int GetClock(int chipset, int freq, int *closestfreq, bool report_error);

void SetClock(int chipset, int freq);

void do_clock(int chipset, int clock, int charwidth);


#endif

