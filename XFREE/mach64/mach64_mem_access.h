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
#ifndef MEM_ACCESS_H
#define MEM_ACCESS_H

/***
 *** mach64_mem_access.h, grants memory access. Mostly used
 *** for fetching BIOS data.
 *** Contributed by M. Grimrath (m.grimrath@tu-bs.de)
 ***/

/*
 * Initializes reading from memory.
 * returns handle to be used by the other functions
 */
int memacc_open(void);

/*
 * Copies 'len' data from (physical?) address 'addr' to userspace
 * 'dst'
 */
void memacc_fetch(int handle, unsigned long addr, void *dst, unsigned len);

/*
 * Deinitializes memory access for the given handle
 */
void memacc_close(int handle);

#endif /* MEM_ACCESS_H */
