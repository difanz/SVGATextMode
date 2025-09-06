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
 *** special SVGA chip functions for SVGATextMode
 ***/

#include "misc.h"
#include "vga_prg.h"
#include "messages.h"  
#include "XFREE/mach64/mach64.h"


void special(int chipset)
/*
 * Chipset specific settings, like memory speed and the likes.
 *
 * Also used to reset some extended registers for better textmode restoration.
 */
{
   int tmp;  

   PDEBUG(("Setting chipset-specific special registers\n"));
   switch(chipset)
   {
     case CS_ATI:
        ATI_PUTEXTREG(0xB0, ATI_GETEXTREG(0xb0) & ~0x08);   /* (for 188xx chips) Enable 8 CRT accesses for each CPU access */
        break;
     case CS_ATIMACH64:
       /*
        * Tries to verify if the hardware specified in the configfile is actually
        * present. Quick hack for Mach64.
        */
        Mach64_verify_and_init();
        break;
     case CS_S3:
        /* set `M-parameter' which controls the DRAM FIFO balancing */
        /* this was derived from the svgalib 1.2.8 code */
        tmp = Inb_CRTC(0x54) & 0x07;
        if (OFLG_ISSET(OPT_XFAST_DRAM)) Outb_CRTC(0x54,tmp | (0 << 3));
        if (OFLG_ISSET(OPT_FAST_DRAM))  Outb_CRTC(0x54,tmp | (2 << 3));
        if (OFLG_ISSET(OPT_MED_DRAM))   Outb_CRTC(0x54,tmp | (10 << 3));
        if (OFLG_ISSET(OPT_SLOW_DRAM))  Outb_CRTC(0x54,tmp | (20 << 3));
        break;
     case CS_ET4000:
     case CS_ET6000:
        /* set to 8/9 bit wide font */
        Outb_SEQ(0x06,0);
        /* disable 16/24/32 bpp modes in case they're on (X-server crash) */
        Outb_ATR_CTL(0x16, 0);
        /* disable Vert/Hor overflow registers (X-server crash) */
        Outb_CRTC(0x35, 0);
        Outb_CRTC(0x3F, 0);
        /* set memory bank pointers back to the first one */
        outb(0, 0x3CD); outb(0, 0x3CB);
        /* set extended linear start address to 0 */
        Outb_CRTC(0x33, 0);
        if (chipset == CS_ET4000)
        {
          /* disable HW cursor on ET4000W32 */
          outb(0xF7, 0x217A); outb(inb(0x217B) & 0x7E, 0x217B);
          outb(0xEC, 0x217A); 
          switch (inb(0x217B) >> 4) {
            case 0:
            case 1:
            case 3:
            case 11: Outb_CRTC(0x30,0x1C); /* linear address map comparator must be 0x1C on pre-W32p chips */
                     break;
            default: Outb_CRTC(0x30,0); /* linear address map comparator must be 0 */
          }
        }
        if (chipset == CS_ET6000)
        {
          /* set System performance control register: FIFO underflow prevention */
          tmp = inb(PCIIOBase+0x41);
          outb(tmp | 0x10, PCIIOBase+0x41);
          /* disable linear memory mode and accel in case it's still on (X-server crash) */
          outb(0, PCIIOBase+0x40);
          /* disable MUX21 */
          outb(0, PCIIOBase+0x42);
          outb(0x0d, PCIIOBase+0x67); outb(0x00, PCIIOBase+0x69);
          /* disable 16/24/32 bpp */
          Outb_ATR_CTL(0x16, 0);
        }
        break;
     case CS_NEOMAGIC:
        if (OFLG_ISSET(OPT_TOPLEFT))
       {
         Outb_GR_CTL( 0x25, 0 );
         Outb_GR_CTL( 0x2f, 0 );
         Outb_GR_CTL( 0x30, 0 );
         Outb_GR_CTL( 0x82, 0 );
       }
       break;
     default:
        PDEBUG(("SPECIAL VGA chip settings: no special settings for chipset #%d\n",chipset));
   }
}


#define HSTEXT_MINCLOCK 36000

void S3_StartHSText_FontLoad(int pixclock, int do_it)
{
   bool hstext = OFLG_ISSET(OPT_S3_HS_TEXT) && (pixclock > HSTEXT_MINCLOCK);
   bool old_was_HS=(Inb_CRTC(0x31) & 0x40);

   PDEBUG(("Current S3 text mode: %s\n", old_was_HS ? "High Speed" : "Normal" ));

   if ( hstext && (!old_was_HS) && !OFLG_ISSET(OPT_LOADFONT) )
   {
      PWARNING(("\n\
       Switching from normal to High Speed text mode requires the Option `LoadFont'.\n\
       Normal text mode will be used until font loading is enabled.\n"));
      hstext=FALSE;
   }
   
   if ( (!hstext) && old_was_HS && !OFLG_ISSET(OPT_LOADFONT) )
   {
      PWARNING(("\n\
       Switching from High Speed to normal text mode requires the Option `LoadFont'.\n\
       High speed text mode will be used until font loading is enabled.\n"));
      hstext=TRUE;
   }
   
   if (do_it)
   {
     Outbit_CRTC(0x3A, 5, 0);   /* normal font store mode, just for sure... */
     if (hstext)
       {
         Outbit_CRTC(0x3A, 5, 1);   /* prepare S3 for high speed font store mode */
         Outbit_CRTC(0x31, 6, 1);   /* enable high speed font fetch mode */
       }
       else
       { 
         Outbit_CRTC(0x31, 6, 0);   /* disable high speed font fetch mode */
       }
   }
}

