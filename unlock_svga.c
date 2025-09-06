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
 *** VGA chip unlocking functions for SVGATextMode
 ***/

#include "misc.h"
#include "vga_prg.h"
#include "messages.h"  


void unlock(int chipset)
{
   int tmp;

   /* unlock ALL locked registers for specified chipset. A bit rough, but simplest */
   PDEBUG(("Unlocking chipset %d\n",chipset));
   UNLOCK_STD_VGA;
   switch(chipset)
   {
    case CS_CIRRUS :
       Outb_SEQ (0x6, 0x12);	/* unlock cirrus special */
       break;
    case CS_S3     : 
       Outb_CRTC(0x39, 0xa5); /* system extension regs (CRTC index 0x50..0x5E) */
       Outb_CRTC(0x38, 0x48); /* S3 register set (CRTC index 0x30..0x3C) */
       Outbit_CRTC(0x35, 4, 0); /* VERT timing regs (CRTC index 6,7(bit0,2,3,5,7),9,10,11(bits0..3),15,16 ) */
       Outbit_CRTC(0x35, 5, 0); /* HOR timing regs (CRTC index 0..5, 17(bit2) ) */
       Outbit_CRTC(0x34, 5, 0); /* bit 0 of Clocking mode reg unlocked (8/9 dot font selection) */
       Outbit_CRTC(0x34, 7, 0); /* Clock bits in MISC reg unlocked */
      /*  Outbit_CRTC(0x40, 0, 1); */ /* enhanced register access, only for access to accelerator commands. Does NOT seem to work on my 805 */
       break;
    case CS_ET4000 :
    case CS_ET3000 :
       outb(0x03, 0x3BF); outb(0xA0, 0x3D8); /* ET4000W32i key */
       break;
    case CS_ATI:
    case CS_ATIMACH32:
    case CS_ATIMACH64:
       ATI_PUTEXTREG(0xB4, ATI_GETEXTREG(0xB4) & 0x03);
       ATI_PUTEXTREG(0xB8, ATI_GETEXTREG(0xb8) & 0xC0);
       ATI_PUTEXTREG(0xB9, ATI_GETEXTREG(0xb9) & 0x7F);
       ATI_PUTEXTREG(0xBE, ATI_GETEXTREG(0xbe) | 0x01);
       break;
    case CS_PVGA1:
    case CS_WDC90C0X:
    case CS_WDC90C1X:
    case CS_WDC90C2X:
    case CS_WDC90C3X:
       Outb_GR_CTL(0x0F, (Inb_GR_CTL(0x0F) & 0x80) | 0x05);
       if (chipset != CS_PVGA1)    /* these might not be needed */
       {
         Outb_CRTC(0x29, (Inb_CRTC(0x29) & 0x70) | 0x85);
         Outb_CRTC(0x2a, Inb_CRTC(0x2a) & 0xF8);
         if (chipset != CS_WDC90C0X)
         {
           Outb_SEQ(0x06, 0x48);
           if (chipset != CS_WDC90C1X) Outb_CRTC(0x34, 0xA6);
         }
       }
       break;
    case CS_ALI:
    case CS_AL2101:
       Outbit_CRTC(0x1A, 4, 1);
       break; 
    case CS_OTI67:
    case CS_OTI77:  /* CS_OTI87 doesn't seem to need unlocking */
       Outb_OTI( OTI_CRT_CNTL, Inb_OTI(OTI_CRT_CNTL) & 0xF0 );
       break; 
    case CS_SIS:
       Outb_SEQ(0x05, 0x86);
       break;
    case CS_ARK:
       Outbit_SEQ(0x1D, 0, 1); 
       break;
    case CS_NCR22E:
    case CS_NCR32:
       Outbit_SEQ(0x05, 0, 1);
       break;
    case CS_MX:
       Outbit_SEQ(0x65, 6, 1); /* not in XFree86, but VGADOC4 says otherwise */
       Outb_SEQ(0xA7, 0x87);
       break;
    case CS_ET6000:
       outb(inb(vgaIOBase+0x08)|0xA0, vgaIOBase+0x08); /* ET6000 KEY register bits */
       break;
    case CS_TGUI:
       tmp = 0x82;
       if (clock_data.clockchiptype >= CLKCHIP_TGUI9440) tmp |= 0x40; /* TGUI9440 or better */
       outw((tmp << 8) | 0x0E, 0x3C4);
       break;
    case CS_NEOMAGIC:
       Outb_GR_CTL( 0x09, 0x26 );
       break;
    default: PDEBUG(("UNLOCK VGA: No special register unlocking needed for chipset #%d\n",chipset));
   }
}
