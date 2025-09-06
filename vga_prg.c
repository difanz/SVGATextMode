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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "misc.h"
#include "vga_prg.h"
#include "messages.h"  
#include "textregs.h" /* for Set_Textmode */

/*
 * this is ugly, it's really declared in sys/io.h but including that here
 * fritzes the inb/outb macros again.  otoh compiler warnings are ugly too.
 *  -- Ron
 */
#ifndef DOS
  extern int iopl(int);
#endif

/*
 * global vars
 */
 
 int vgaIOBase  = 0x3D0; /* default = color */
 unsigned long PCIIOBase  = 0xDEADBEEF;
 int vgaCRIndex = 0x3D4;
 int vgaCRReg   = 0x3D5;
 bool vga_open   = FALSE;

/*
 * Some functions to make life easier (?!)
 */

void Outb_ATR_CTL (int index, int data)
{
  inb(ATR_CTL_INDEX_DATA_SWITCH);
  outb (index & 0x1f, ATR_CTL_INDEX);
  outb ( data, ATR_CTL_DATA_W);
  inb(ATR_CTL_INDEX_DATA_SWITCH);
  outb ( (index & 0x1f) | 0x20, ATR_CTL_INDEX);
  outb ( data, ATR_CTL_DATA_W);
}

int inb_ATR_CTL (int index)
{
  int res;

  inb(ATR_CTL_INDEX_DATA_SWITCH);
  outb (index & 0x1f, ATR_CTL_INDEX);
  res=inb(ATR_CTL_DATA_R);
  inb(ATR_CTL_INDEX_DATA_SWITCH);
  outb ( (index & 0x1f) | 0x20, ATR_CTL_INDEX);
  inb(ATR_CTL_DATA_R);
  return res;
}


/*****************************************************************************************************************************/

#ifdef PARANOID_IO_PERMIT

void get_IO_range(int start, int len)
{
  PDEBUG(("Getting VGA IO permissions for addr. range 0x%x-0x%x\n", start, start+len-1));
  if ( (start >= 0x400) || ((start+len) >= 0x400) )
  {
    if (iopl(3) != 0)
    {
      perror("I/O Privilege permissions");
      PERROR(("Cannot set I/O privileges.\n\
               You must be root, or the program must be setuid root!\n"));
    }
  }
  else
  {
    if (ioperm(start, len, 1) != 0)
    {
      perror("VGA I/O Permissions");
      PERROR(("Cannot get I/O permissions for hardware address range 0x%x-0x%x.\n\
               You must be root, or the program must be setuid root!\n", start, start+len-1));
    }
  }
}

/*
 * Get IO permissions, and setup some global variables for VGA programming.
 */

void get_VGA_io_perm(int chipset)
{
  PDEBUG(("Getting VGA IO permissions for chipset #%d\n", chipset));
  vga_open=FALSE;
  get_IO_range(0x3b4, 0x3df - 0x3b4 + 1);
  switch(chipset)
  {
    case CS_ATI:
    case CS_ATIMACH32:
       get_IO_range(ATI_EXTREG, 2);
       break;
    case CS_S3:
       get_IO_range(0x200, 2);   /* used by ICD2061 clock code for IOdelay */
                                 /* 0x80 is a lot safer (Linux uses it), since there
                                    can be hardware at that address -- but we'll
                                    stick with it for compatibility */
       break;
    case CS_ET4000:
       get_IO_range(0x217A, 2);
       break;
    case CS_ET6000:
       PCIIOBase = (Inb_CRTC(0x21)<<8);
       PCIIOBase += (Inb_CRTC(0x22)<<16);
       PCIIOBase += (Inb_CRTC(0x23)<<24); /* keep this split up */
       PDEBUG(("ET6000 PCI config space: 0x%X\n", PCIIOBase));
       get_IO_range(PCIIOBase, 256);
       PDEBUG(("PCI vendor ID = 0x%X ; device ID = 0x%X\n", inw(PCIIOBase), inw(PCIIOBase+2)));
       break;
    case CS_TGUI:
       get_IO_range(0x43C6, 4);
       break;
    case CS_MATROX:
    case CS_RIVA128:
       get_IO_range(0x400, 1); /* dummy: will give us access to all IO addresses >= 0x400 */
       break;
    case CS_ATIMACH64:
       get_IO_range(0,0xFFFF); /* it's hard to know where IO will happen on MACH64 */
       break;
  }
  
#ifdef RUN_SECURE
  Renounce_SUID; /* if we are Setuid Root: renounce to further superuser rights (safer) */
#endif

  vga_open=TRUE;  /* needed for PERROR, so it knows if it can restore the screen or not */

  /* this is probably not the best place to put this */
  vgaIOBase  = GET_VGA_BASE;
  vgaCRIndex = (vgaIOBase+4);
  vgaCRReg   = (vgaIOBase+5);
}

#else  /* let's be blunt :-) */

void get_IO_range(int start, int len)
{
  if (iopl(3) != 0)
  {
    perror("I/O Privilege permissions");
    PERROR(("Cannot set I/O privileges.\n\
             You must be root, or the program must be setuid root!\n"));
  }

}
void get_VGA_io_perm(int chipset)
{
  PDEBUG(("Getting IO permissions\n"));
  vga_open=FALSE;
  get_IO_range(0x400,1);
#ifdef RUN_SECURE
  Renounce_SUID; /* if we are Setuid Root: renounce to further superuser rights (safer) */
#endif

  vga_open=TRUE;  /* needed for PERROR, so it knows if it can restore the screen or not */

  /* this is probably not the best place to put this */
  if (chipset == CS_ET6000)
  {
     PCIIOBase = (Inb_CRTC(0x21)<<8);
     PCIIOBase += (Inb_CRTC(0x22)<<16);
     PCIIOBase += (Inb_CRTC(0x23)<<24); /* keep this split up */
     PDEBUG(("ET6000 PCI config space: 0x%X\n", PCIIOBase));
     PDEBUG(("PCI vendor ID = 0x%X ; device ID = 0x%X\n", inw(PCIIOBase), inw(PCIIOBase+2)));
  }
  vgaIOBase  = GET_VGA_BASE;
  vgaCRIndex = (vgaIOBase+4);
  vgaCRReg   = (vgaIOBase+5);
}
#endif

/*****************************************************************************************************************************/

void interlace(int chipset, t_mode *m)
/*
 * chipset specific interlace settings (also used to un-set interlace mode)
 * some chipsets need to change some timings for interlacing (e.g. dividing V-timings by 2).
 */
{
   int il = MOFLG_ISSET(m, ATTR_INTERLACE);
   
   /* PDEBUG(("%sing interlacing mode\n", il ? "Sett" : "Disabl")); */
   switch(chipset)
   {
     case CS_S3    : Outbit_CRTC(0x42, 5, il ? 1 : 0);
                     if (il)
                     {
                       m->VDisplay /= 2;
                       m->VSyncStart /= 2;
                       m->VSyncEnd /= 2;
                       m->VTotal /= 2;
                       m->VBlankStart /= 2;
                       m->VBlankEnd /= 2;
                       Outb_CRTC(0x3C, m->HDisplay/2);
                     }
                     break;
     default: if (il) PDEBUG(("INTERLACING not supported yet on chipset #%d\n",chipset));
   }
}
/*****************************************************************************************************************************/

void set_V_timings(int active, int start_sync, int stop_sync, int total)
{
  int vbs, vbe;
  if (total > 1023)
  {
    /* round UP around sync to avoid null sync width */
    active = active / 2;
    start_sync = start_sync / 2;
    stop_sync = (stop_sync + 1) / 2;
    total = (total + 1) / 2;
    Outbit_CRTC(0x17, 2, 1);  /* Vertical total double mode */
  }
  else Outbit_CRTC(0x17, 2, 0);
  Set_VERT_TOTAL(total);
  Set_VDISPL_END(active);
  Set_VRETRACE_START(start_sync); Set_VRETRACE_END(stop_sync);
  /* set 8 lines of overscan, the rest is blanking (if the sync doesn't come too close) */
  vbs = MIN(start_sync, active+8);
  vbe = MAX(stop_sync, total-8);
  if ((vbe-vbs)>=127)
   {
     PDEBUG(("V Blanking size >= 127 ; setting to 127\n"));
     vbe = vbs+127;
   }
  Set_VBLANK_START(vbs); Set_VBLANK_END(vbe);
}

void set_H_timings(int active, int start_sync, int stop_sync, int total)
{
  int hrs=start_sync/8;
  int hre=stop_sync/8;
  int hde=(active/8) & 0xFFFFFFFE;
  int htot=total/8;

  /* set 1 char of overscan, with the rest blanked (if sync positions don't overlap) */
  int hbs=MIN(hrs, hde+1);
  int hbe=MAX(hre, htot-1);
  if ((hbe-hbs)>=63)
   {
     PDEBUG(("H Blanking size >= 63 ; setting to 63\n"));
     hbe = hbs+63;
   }
  Set_HOR_TOTAL(htot);
  Set_HOR_DISPL_END(hde);
  Set_HSYNC_START(hrs); Set_HSYNC_END(hre);
  Set_LOG_SCREEN_WIDTH(hde);
  Set_HBLANK_START(hbs); Set_HBLANK_END(hbe);
}

int set_charwidth(int width)
{
   switch(width)
   {
      case 8: Outbit_SEQ(1, 0, 1);
              SET_PIXELPAN(0);
              break;
      case 9: Outbit_SEQ(1, 0, 0);
              SET_PIXELPAN(8);
              Outbit_ATR_CTL(0x10, 2, OFLG_ISSET(OPT_ISO_FONT9) ? 0 : 1);
              break;
      default: return(1);
   }
   return(0);
}

void Set_Textmode(bool set_all)
{
  int i;

  if (set_all)  /* set80 functionality: restores _all_ standard VGA registers to known state */
  {
    PDEBUG(("Setting ALL standard VGA registers to known state\n"));
    outb(TXT_MISC_REG, VGA_MISC_W);
    SYNCRESET_SEQ;
    for (i=0 ; i<NUM_STD_SEQ_REGS; i++) Outb_SEQ(i, TXT_SEQ_REGS[i]);
    ENDRESET_SEQ;
    for (i=0 ; i<NUM_STD_CRTC_REGS; i++) Outb_CRTC(i, TXT_CRTC_REGS[i]);
    for (i=0 ; i<NUM_STD_GRCTL_REGS; i++) Outb_GR_CTL(i, TXT_GRCTL_REGS[i]);
    for (i=0 ; i<NUM_STD_ATRCTL_REGS; i++) Outb_ATR_CTL(i, TXT_ATRCTL_REGS[i]);
    for (i=0 ; i<64; i++) WRITERGB(i, STD_PALETTE[i][0], STD_PALETTE[i][1], STD_PALETTE[i][2]);
    for (i=64 ; i<256; i++) WRITERGB(i, 0, 0, 0);
  }
  else
  {
    Outb_GR_CTL(6,Inb_GR_CTL(6) & 0xFE);
#ifndef DOS
    for (i=0 ; i<16; i++) Outb_ATR_CTL(i, TXT_ATRCTL_REGS[i]); /* ensure palette is mapped as Linux expects */
#endif
    Outb_ATR_CTL(16,inb_ATR_CTL(16) & 0xFE);
    Outb_GR_CTL(0x08,0xFF); /* pixel bit mask */
  }
}


/*****************************************************************************************************************************/
/* "get" functions: used for getting certain parameters from the chips (for grabmode etc.) */

inline int get_charwidth()
{
  return((Inb_SEQ(1) & 0x01) ? 8 : 9);
}


inline int Get_VERT_TOTAL()
{
  return( (Inb_CRTC(0x6) + ((Inb_CRTC(0x7) & 0x01) ? 256 : 0) + ((Inb_CRTC(0x7) & 0x20) ? 512 : 0)) + 2);
}


inline int Get_HOR_TOTAL()
{
  return(Inb_CRTC(0)+5);
}


inline int Get_HOR_DISPL_END()
{
  return((int)Inb_CRTC(1)+1);
}

/* calculate end value from start value and mask. this is because "end" values in VGA regs define only the few
 * last bits of the entire value. See VGA data for more */
 
#define ENDCALC(start,end,mask) ( ((((start & mask) > end) ? (start + (mask+1)) : start) & ~mask) | end  )

inline int Get_HSYNC_START()
{
  return((int)Inb_CRTC(4));
}

inline int Get_HSYNC_END()
{
  int start, end;
  start = Get_HSYNC_START();
  end = Inb_CRTC(5) & 0x1f;
  return(ENDCALC(start, end, 0x1f));
}

inline int Get_HBLANK_START()
{
  return((int)Inb_CRTC(2) + 1);
}

inline int Get_HBLANK_END()
{
  /* this does not work correctly when Get_HOR_TOTAL() reads wrapped data (as in 2048+ pixel wide modes) */
  int start, end, htot, endblk;
  htot = Get_HOR_TOTAL();
  start = Get_HBLANK_START();
  end = ((int)Inb_CRTC(3) & 0x1F) + ((Inb_CRTC(5) & 0x80) ? 0x20 : 0);
  endblk = ENDCALC(start, end, 0x3f)+1;
  return((endblk < htot) ? endblk : (end+1));
}

inline int Get_VBLANK_START()
{
  return((int)Inb_CRTC(0x15) + ((Inb_CRTC(0x7) & 0x08) ? 256 : 0) + ((Inb_CRTC(0x9) & 0x20) ? 512 : 0));
}

inline int Get_VBLANK_END()
{
  int start = Get_VBLANK_START()-1;
  int end = Inb_CRTC(0x16);
  return(ENDCALC(start, end, 0xff));
}
 
 
inline int Get_VERT_DISPL_END()
{
  return( ( (int)Inb_CRTC(0x12) + ((Inb_CRTC(0x7) & 0x02) ? 256 : 0) + ((Inb_CRTC(0x7) & 0x40) ? 512 : 0) ) +1);
}

inline int Get_VRETRACE_START()
{
  int vrs = (int)Inb_CRTC(0x10) + ((Inb_CRTC(0x7) & 0x04) ? 256 : 0) + ((Inb_CRTC(0x7) & 0x80) ? 512 : 0);
  return( (vrs==0) ? 1024 : vrs );
}

inline int Get_VRETRACE_END()
{
  int start, end;
  start = Get_VRETRACE_START();
  end = Inb_CRTC(0x11) & 0x0f;
  return(ENDCALC(start, end, 0x0f));
}

inline int Get_MAX_SCANLINE()
{
  return((Inb_CRTC(0x9) & 0x1f) +1);
}

inline int Get_HSYNC_POLARITY()
{
  return( (inb(VGA_MISC_R) & 0x40) ? NEG : POS);
}

inline int Get_VSYNC_POLARITY()
{
  return( (inb(VGA_MISC_R) & 0x80) ? NEG : POS);
}

inline int Get_TX_GR_Mode()
/* returns 0 for text mode, 1 for graphics mode */
{
  return(Inb_GR_CTL(6) & 0x01);
}

inline int Get_HORIZ_SHIFT()
{
  return( (Inb_CRTC(5) & 0x60) >> 5 );
}

