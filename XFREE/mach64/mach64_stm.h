/*  SVGATextMode -- An SVGA textmode manipulation/enhancement tool
 *
 *  Copyright (C) 1995,1996  Koen Gadeyne
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
 *** translation stuff between original MACH64 XFree 3.2 code and
 *** SVGATextMode syntax.
 ***/
 
#ifndef _MACH64_STM_H
#define _MACH64_STM_H

#include "../../chipset.h"

#define mach64MinFreq ((unsigned short)(ClockchipData[clock_data.clockchiptype].minclock/10))
#define mach64MaxFreq ((unsigned short)(ClockchipData[clock_data.clockchiptype].maxclock/10))
#define mach64RefFreq ((unsigned short)(clock_data.refclk/10))

extern unsigned mach64IOBase;

#endif
