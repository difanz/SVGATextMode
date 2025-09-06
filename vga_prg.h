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
 *** VGA chip programming functions for SVGATextMode
 ***/

#ifndef _VGA_PRG_H
#define _VGA_PRG_H

#include "misc.h"

extern int vgaIOBase;
extern unsigned long PCIIOBase;
extern bool vga_open;

#include "chipset.h"

#if defined(WIN) || defined(DOS)
#  include "asm/io.h"    /* this assumes the CYGNUS GCC for Windows, \
                          which seems to lack IO instructions, or the
                          gcc-dos cross-compiler */
#  define ioperm(x,y,z) (0)
#  define iopl(x) (0)
#else
#  include <asm/io.h>
#  ifdef __ALPHA_IO_H
#    define _ASM_IO_H
#  endif
#endif

#ifndef _ASM_IO_H
#  ifdef DJGPP
#    include <pc.h>
#    define outb(data,port) outportb(port,data)
#    define outw(data,port) outportw(port,data)
#    define inb(port) inportb(port)
#    define inw(port) inportw(port)
#  else /* Borland C */
#    define outb(data,port) outp(port,data)
#    define outw(data,port) outpw(port,data)
#    define inb(port) inp(port)
#    define inw(port) inpw(port)
#  endif
#  define ioperm(x,y,z) (0)
#  define iopl(x) (0)
#endif

/*
 * Some important VGA registers
 */

#define SEQ_INDEX	0x3C4
#define SEQ_DATA	(SEQ_INDEX+1)
#define CRTC_INDEX	(vgaIOBase+0x04)
#define CRTC_DATA	(CRTC_INDEX+1)
#define VGA_MISC_R	0x3CC
#define VGA_MISC_W	0x3C2
#define VGA_FEAT_R	0x3CA
#define VGA_FEAT_W	(vgaIOBase+0x0A)
#define GR_CTL_INDEX	0x3CE
#define GR_CTL_DATA	(GR_CTL_INDEX+1)
#define ATR_CTL_INDEX	0x3C0
#define ATR_CTL_DATA_W	(ATR_CTL_INDEX)
#define ATR_CTL_DATA_R	(ATR_CTL_INDEX+1)
#define STATUS1		(vgaIOBase+0x0A)
#define STATUS0		0x3C2
#define ATR_CTL_INDEX_DATA_SWITCH  STATUS1
#define DAC_MASK        0x3C6
#define DAC_STATUS	0x3C7
#define DAC_RD_INDEX	0x3C7
#define DAC_WR_INDEX    0x3C8
#define DAC_DATA	0x3C9

/*
 * Simplified VGA access instruction macro's
 */

#define GET_VGA_BASE    ( (inb(0x03CC) & 0x01) ? 0x03D0 : 0x03B0 )

#define Outb_SEQ(index, data)   ( outw( (data) << 8 | (index), SEQ_INDEX) )
#define Inb_SEQ(index)          ( outb(index, SEQ_INDEX) , inb(SEQ_DATA) )

#define Outb_CRTC(index, data)  ( outw( (data) << 8 | (index), CRTC_INDEX) )
#define Inb_CRTC(index)         ( outb(index, CRTC_INDEX) , inb(CRTC_DATA) )
 
#define Outb_GR_CTL(index, data) ( outw( (data) << 8 | (index), GR_CTL_INDEX) )
#define Inb_GR_CTL(index)        ( outb(index, GR_CTL_INDEX) , inb(GR_CTL_DATA) )

#define OutRGB_DAC(index, data)   ( outb(index, DAC_WR_INDEX) , outb((data >> 16) & 0xFF, DAC_DATA) ,\
                                                              outb((data >> 8 ) & 0xFF, DAC_DATA) ,\
                                                              outb((data      ) & 0xFF, DAC_DATA) )
#define InRGB_DAC(index)          ( outb(index, DAC_RD_INDEX) , ( ( (int) (inb(DAC_DATA) << 8) | inb(DAC_DATA)) << 8) | inb(DAC_DATA) )
 
#define Mask(x)  (1<<(x)) 
#define SetClearBit(indata, bitno, setclear)  ( ((indata) & ~Mask(bitno)) | (((setclear) == 0) ? 0 : Mask(bitno)) )
 
#define Outbit_SEQ(index, bitno, data)   ( Outb_SEQ (index, SetClearBit(Inb_SEQ(index), bitno, data)) )
#define Outbit_CRTC(index, bitno, data)  ( Outb_CRTC (index, SetClearBit(Inb_CRTC(index), bitno, data)) )
#define Outbit_ATR_CTL(index, bitno, data)  ( Outb_ATR_CTL (index, SetClearBit(inb_ATR_CTL(index), bitno, data)) )
#define Outbit_GR_CTL(index, bitno, data)  ( Outb_GR_CTL (index, SetClearBit(Inb_GR_CTL(index), bitno, data)) )

#define WRITERGB(index,r,g,b) ( outb((index),DAC_WR_INDEX) , outb((r),DAC_DATA) , outb((g),DAC_DATA) , outb((b),DAC_DATA) )


void Outb_ATR_CTL (int index, int data);

int inb_ATR_CTL (int index);

#define SET_PIXELPAN(no)        ( Outb_ATR_CTL(0x13, (inb_ATR_CTL(0x13) & 0xf0) | ((no) & 0x0f)) )

#define SCREEN_OFF	( Outbit_SEQ(0x1, 5, 1) )
#define SCREEN_ON	( Outbit_SEQ(0x1, 5, 0) )
#define WAIT_VERT_BLK	( while(inb(0x3c2) & 0x80) )

#if DOSYNCRESET
#  define SYNCRESET_SEQ	( Outbit_SEQ(0, 1, 0) ) /* to avoid memory corruption when changing clocks */
#  define ENDRESET_SEQ	( Outb_SEQ(0, Inb_SEQ(0) | 0x03) )
#else
#  define SYNCRESET_SEQ 
#  define ENDRESET_SEQ
#endif

/* ATI special registers */
#define ATI_EXTREG      0x1CE
#define ATI_GETEXTREG(Index)         ( outb((Index), ATI_EXTREG), inb(ATI_EXTREG+1) )
#define ATI_PUTEXTREG(Index, data)   ( outw((data)<<8|(Index), ATI_EXTREG) )

/* OTI special registers */

#define OTI_INDEX       0x3DE             /* Oak extended index register */
#define OTI_R_W         OTI_INDEX+1       /* Oak extended r/w register */
#define OTI87_CLOCK     0x6               /* r/w Selects from 16 clocks */
#define OTI_CRT_CNTL    0xC               /* Oak CRT COntrol Register */
#define OTI_MISC        0xD               /* Oak Misc register */
#define Outb_OTI(index, data)  ( outw( (data) << 8 | (index), OTI_INDEX) )
#define Inb_OTI(index)         ( outb(index, OTI_INDEX) , inb(OTI_R_W) )
#define Outbit_OTI(index, bitno, data)  ( Outb_OTI (index, SetClearBit(Inb_OTI(index), bitno, data)) )

/* standard VGA unlocking */
#define UNLOCK_STD_VGA   Outbit_CRTC (0x11, 7, 0) /* CRTC index 0x00..0x07 */


/*****************************************************************************************************************************/
void get_VGA_io_perm(int chipset);
void get_IO_range(int start, int len);

#ifndef DOS
#define Renounce_SUID     ( setuid(getuid()) , setgid(getgid()) )
#else
#define Renounce_SUID
#endif

/*****************************************************************************************************************************/

void special(int chipset);

void interlace(int chipset, t_mode *m);


void set_V_timings(int active, int start_sync, int stop_sync, int total);

void set_H_timings(int active, int start_sync, int stop_sync, int total);

int set_charwidth(int width);
int get_charwidth();

/*
 * Some more general functions (less low-level than separate register access)
 */

#define Set_VERT_TOTAL(vt)  (Outb_CRTC (0x6, ((vt)-2) & 0xff) , Outbit_CRTC(0x7, 0, ((vt)-2) & 0x100) , Outbit_CRTC(0x7, 5, ((vt)-2) & 0x200))
inline int Get_VERT_TOTAL();

#define Set_VRETRACE_START(start)  (Outb_CRTC (0x10, (start) & 0xff) , Outbit_CRTC (0x7, 2, (start) & 0x100) , Outbit_CRTC (0x7, 7, (start) & 0x200) )
inline int Get_VRETRACE_START();
#define Set_VRETRACE_END(end)      (Outb_CRTC (0x11, (Inb_CRTC (0x11) & 0xf0) | ((end) & 0xf)) )
inline int Get_VRETRACE_END();

#define Set_VBLANK_START(start)  (Outb_CRTC (0x15, (start) & 0xff) , Outbit_CRTC(0x7, 3, (start) & 0x100) , Outbit_CRTC(0x9, 5, (start) & 0x200) )
#define Set_VBLANK_END(end)      (Outb_CRTC (0x16, (end) & 0xFF) )
inline int Get_VBLANK_START();
inline int Get_VBLANK_END();

#define Set_VDISPL_END(vde)  (Outb_CRTC (0x12, ((vde) -1) & 0xff) , Outbit_CRTC(0x7, 1, ((vde) -1) & 0x100) , Outbit_CRTC(0x7, 6, (((vde) -1) & 0x200)))
inline int Get_VERT_DISPL_END();

#define Set_MAX_SCANLINE(msl)  (Outb_CRTC (0x9, (Inb_CRTC (0x9) & 0xe0) | (((msl)-1) & 0x1f)))
inline int Get_MAX_SCANLINE();

#define Set_CURSOR_START(start)  (Outb_CRTC (0x0A,  (Inb_CRTC(0x0a) & 0xe0) | ((start) & 0x1f) ))
#define Set_CURSOR_END(end)      (Outb_CRTC (0x0B,  (Inb_CRTC(0x0b) & 0xe0) | ( (end)  & 0x1f) ))

#define Set_HOR_TOTAL(htot)  (Outb_CRTC(0, (htot) - 5))
inline int Get_HOR_TOTAL();

#define Set_HOR_DISPL_END(hend)  (Outb_CRTC(1, (hend) - 1))
inline int Get_HOR_DISPL_END();

#define Set_HSYNC_START(start)  (Outb_CRTC(4, start))
#define Set_HSYNC_END(end)  (Outb_CRTC(5, (Inb_CRTC(5) & 0xe0) | ((end) & 0x1f)))
inline int Get_HSYNC_START();
inline int Get_HSYNC_END();

#define Set_HBLANK_START(start)  (Outb_CRTC(2, (start) - 1))
#define Set_HBLANK_END(end)  (Outb_CRTC(3, (Inb_CRTC(3) & 0xe0) | (((end) -1) & 0x1f)) , Outbit_CRTC(5, 7, ((end) -1) & 0x20))
inline int Get_HBLANK_START();
inline int Get_HBLANK_END();

#define Set_HSYNC_POLARITY(hpol)  (outb((inb(VGA_MISC_R) & 0xBF) | (((hpol) == NEG) ? 0x40 : 0x00) , VGA_MISC_W))
inline bool Get_HSYNC_POLARITY();
#define Set_VSYNC_POLARITY(vpol)  (outb((inb(VGA_MISC_R) & 0x7F) | (((vpol) == NEG) ? 0x80 : 0x00) , VGA_MISC_W))
inline bool Get_VSYNC_POLARITY();

#define Set_LOG_SCREEN_WIDTH(width)  (Outb_CRTC(0x13, (width)/2))
#define Get_LOG_SCREEN_WIDTH  ((int)Inb_CRTC(0x13)*2)

void Set_Textmode();
#define Set_Graphmode  (Outb_GR_CTL(6,Inb_GR_CTL(6) | 0x01) , Outb_ATR_CTL(16,inb_ATR_CTL(16) | 0x01))

inline int Get_TX_GR_Mode();

#define Set_HORIZ_SHIFT(hshift) (Outb_CRTC(5, (Inb_CRTC(5) & ~0x60) | (hshift << 5)))
inline int Get_HORIZ_SHIFT();

#define Set_Underline_location(line)  (Outb_CRTC(0x14, line))

#endif

