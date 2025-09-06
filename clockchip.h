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
 *** SVGA clockchip programming functions for SVGATextMode
 ***/

#ifndef _CLOCKCHIP_H
#define _CLOCKCHIP_H

typedef unsigned char byte;

#include "XFREE/xfree_compat.h"
#include "XFREE/common_hw/xf86_HWlib.h"

/*
 * Supported ClockChips
 */
 
void set_clockchip_clock(int chipset, long freq, int charwidth);

void set_clockchip_Mclock(int chipset, long freq);

#endif

