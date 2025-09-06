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
#ifndef MACH64_H
#define MACH64_H

/***
 *** mach64.h, mode validation functions for mach64 cards
 *** Contributed by M. Grimrath (m.grimrath@tu-bs.de)
 ***/

/*
 * Reads some values from BIOS, print warnings if they don't match with
 * the Config file.
 * 
 * Assumes access to all ioports has been granted!
 */
void Mach64_verify_and_init(void);

/*
 * Programs the textclock. It is clever enough to determine the clock #
 * which is used for the current textmode.
 * 
 * 'freq' is kHz
 * 
 * Assumes access to all ioports has been granted!
 */
void Mach64_SetClock(int freq, int charwidth);

#endif /* MACH64_H */
