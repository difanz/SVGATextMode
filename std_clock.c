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
 *** SVGA clock programming functions for SVGATextMode
 ***
 *** Almost all this code is adapted from the XFree86 3.1.1 or 3.1.2 code
 *** See the XFree86 distribution for copyrights and authors
 ***/

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "misc.h"
#include "vga_prg.h"
#include "std_clock.h"
#include "clockchip.h"
#include "messages.h"  

/* bits 0 and 1 of clock no */
void SET_CLOCKBITS_0_1(int no)
{
  unsigned char tmp = inb(VGA_MISC_R);
  tmp = (tmp & 0xf3) | ((no & 0x03) << 2);
/*  PDEBUG(("clock no = %d; tmp = 0x%x\n", no, tmp));*/
  outb(tmp , VGA_MISC_W);
}

void LegendClockSelect(int no)
 /* used in both S3 and ET4000 cards (according to XFREE 3.1.1) */
{
  PDEBUG(("Using Sigma Legend clock selection\n"));
  outb((inb(VGA_MISC_R) & 0xF3) | ((no & 0x10) >> 1) | (no & 0x04), VGA_MISC_W);
  Outb_CRTC(0x34, 0x00);
  Outb_CRTC(0x34, 0x02);
  Outb_CRTC(0x34, (no & 0x08) >> 2);
  SET_CLOCKBITS_0_1(no);
}

void TGUIClockSelect(int no)
{
  int tridentReprogrammedMCLK = 0;
  int MCLK_A, MCLK_B, num;

  if (OFLG_ISSET(OPT_FAST_DRAM | OPT_MED_DRAM | OPT_SLOW_DRAM))
  {
    /* set DRAM speed (MCLK) */
    if (clock_data.clockchiptype >= CLKCHIP_TGUI9440)  /* TGUI9440AGi or better */
    {
      if (OFLG_ISSET(OPT_FAST_DRAM))  tridentReprogrammedMCLK = 0x00AF; /* 80MHz, 45ns */
      if (OFLG_ISSET(OPT_MED_DRAM))   tridentReprogrammedMCLK = 0x0307; /* 58MHz, 70ns */
      if (OFLG_ISSET(OPT_SLOW_DRAM))  tridentReprogrammedMCLK = 0x02C6; /* 50MHz, 80ns */
      MCLK_A = tridentReprogrammedMCLK & 0x00FF;
      MCLK_B = (tridentReprogrammedMCLK & 0xFF00) >> 8;
      outb(MCLK_A, 0x43C6);
      outb(MCLK_B, 0x43C7);
    }
    else
    {
      if (OFLG_ISSET(OPT_FAST_DRAM))  tridentReprogrammedMCLK = 0xFF00;
      if (OFLG_ISSET(OPT_MED_DRAM))   tridentReprogrammedMCLK = 0xFF40;
      if (OFLG_ISSET(OPT_SLOW_DRAM))  tridentReprogrammedMCLK = 0xFF50;
      MCLK_A = tridentReprogrammedMCLK & 0x00FF;
      MCLK_B = 0x03;
      Outb_CRTC(0x2C, MCLK_A);
      Outb_CRTC(0x2B, MCLK_B);
    }
  }

  if (clock_data.clockchiptype==CLKCHIP_NONE) /* not programmable */
	  num = clock_data.num_clocks;
  else
	  num = 4;
	
  TVGAClockSelect(CS_TVGA8900, num, no);
}

void TVGAClockSelect(int chipset, int num_clocks, int no)
{
    PDEBUG(("TVGAClockSelect: clock no %d (of total %d) for chipset %d.\n", no, num_clocks, chipset));
    SET_CLOCKBITS_0_1(no);
    Outb_SEQ(0x0B, 0); Inb_SEQ(0x0B);  /* Select "new mode regs" for CS2 and TVGA9000 CS3. */
#if 0
    Outb_SEQ(0x0D, Inb_SEQ(0x0D) & 0xF9);  /* set clock dividers to no division */
#else
    /* according to Michael Karcher, the clock division bits are as follows:
     * Bit0  Bit1    Divisor
     *  0     0         1
     *  1     0         2
     *  0     1         4
     *  1     1        1.5
     */
    /* set clock dividers to no division */
    Outbit_SEQ(0x0D, 1, 0);
    Outbit_SEQ(0x0D, 2, 0);
#endif    
    Outbit_SEQ(0x0D, 0, no & 0x04);  /* bit 2 of clock no */
    if (chipset == CS_TVGA9000)
    {
       Outbit_SEQ(0x0D, 6, no & 0x08);  /* bit 3 of clock no */
    }
    if ((chipset == CS_TVGA8900) && (num_clocks > 7))
    {
       Outb_SEQ(0x0B, 0);    /* Switch to Old Mode for CS3 */
       Outbit_SEQ(0x0E, 4, no & 0x08);      /* bit 3 of clock no */
    }
}


void s3ClockSelect(int no)
{
   if (OFLG_ISSET(OPT_LEGEND)) LegendClockSelect(no);
   else
   {
     if (no<2) /* prefer standard VGA clock registers -- needed for S3Trio */
     {
       SET_CLOCKBITS_0_1(no);
     }
     else
     {
       SET_CLOCKBITS_0_1(3);    /* select clock #3 to allow extra clocks to be used */
       Outb_CRTC(0x42, no & 0x0f);
     }
   }
}


void ATIClockSelect(int chipset, int no)
{
   /* for MACH32 chipsets, bits 2 and 3 of the clock selection number must be
      exchanged to comply with the XFREE MACH server clock ordering.
      Derived through experimentation */
   if (chipset==CS_ATIMACH32) no = ( no & ~0xc) | ((no & 0x04)<<1) | ((no & 0x08)>>1);
   ATI_PUTEXTREG(0xBE, (ATI_GETEXTREG(0xBE) & 0xEF) | ((no << 2) & 0x10));  /* clock bit 2 */
   ATI_PUTEXTREG(0xB9, (ATI_GETEXTREG(0xB9) & 0xFD) | ((no >> 2) & 0x02));  /* clock bit 3 */
   ATI_PUTEXTREG(0xB8, (ATI_GETEXTREG(0xB8) & 0x3F) | ((no << 2) & 0xC0));  /* clock divider */
   SET_CLOCKBITS_0_1(no);    /* clock bits no 0 and 1 , must be set LAST */
 }


void WDCClockSelect(int chipset, int num_clocks, int no)
{
  int MClkIndex = 8;
  int save_cs2 = 0;

  if ((chipset == CS_WDC90C1X) || (chipset == CS_WDC90C2X) || (chipset ==CS_WDC90C3X))
    if (OFLG_ISSET(OPT_SWAP_HIBIT)) save_cs2 = 0; else save_cs2 = 4;
  else
    if (OFLG_ISSET(OPT_SWAP_HIBIT)) save_cs2 = 4; else save_cs2 = 0;

  if (num_clocks > 8) MClkIndex = num_clocks - 1;

  if ((no == MClkIndex) && (chipset != CS_PVGA1))
  {
    /*
     * On all of the chipsets after PVGA1, you can feed MCLK as VCLK.
     */
    PDEBUG(("WDCClockSelect: Selecting MCLOCK as VCLOCK\n"));
    Outbit_CRTC(0x2E, 4, 1);
  }
  else
  {
    /*
     * Disable feeding MCLK to VCLK
     */
    if (chipset != CS_PVGA1) Outbit_CRTC(0x2E, 4, 0);

    SET_CLOCKBITS_0_1(no);
    
  if ((chipset != CS_PVGA1) && (chipset != CS_WDC90C0X))
    Outbit_GR_CTL(0x0C, 1, (no & 0x04) ^ save_cs2); /* bit 2 of clock no */ 
  if (chipset==CS_WDC90C3X)
    Outbit_SEQ(0x12, 2, (no & 0x08) ^ 0x08); /* inverse clock bit 3 */
  }
}


void ET4000ClockSelect(int no)
{
   int hibit = 1;
   if (OFLG_ISSET(OPT_HIBIT_HIGH)) hibit = 1;
   if (OFLG_ISSET(OPT_HIBIT_LOW)) hibit = 0;
   PDEBUG(("ET4000 Clock selection: hibit = %d\n", hibit));
   /* program clock */
   if (OFLG_ISSET(OPT_LEGEND)) LegendClockSelect(no);
   else
   {
     SET_CLOCKBITS_0_1(no);
     Outbit_CRTC(0x34, 1, no & 0x04);                                   /* bit 2 of clock no */
     if (!OFLG_ISSET(OPT_ET4000_ALTCLK))
     {
        Outbit_SEQ(7, 6, (hibit == 0) ? (no & 0x08) : (no & 0x08)^0x08);   /* bit 3 of clock no */
        Outbit_CRTC(0x31, 6, no & 0x10); /* bit 4 of clock no */
     }
     else
     {
       PDEBUG(("ET4000 clock selection: Using alternate clock selection mechanism\n"));
       Outbit_SEQ(7, 6, 0); Outbit_SEQ(7, 0, 0); /* just to make sure : disable clock/2 and clock/4 */
       Outbit_CRTC(0x31, 6, (hibit == 0) ? (no & 0x08) : (no & 0x08)^0x08);   /* bit 3 of clock no */
       Outbit_CRTC(0x31, 7, no & 0x10);    /* bit 4 of clock no */
     }
   }
}

void ET6000ClockSelect(int no)
{
     SET_CLOCKBITS_0_1(no);
     Outbit_CRTC(0x34, 1, no & 0x04); /* bit 2 of clock no */
}

void ET3000ClockSelect(int no)
{
  SET_CLOCKBITS_0_1(no);
  Outbit_CRTC( 0x24, 1, no & 0x04); /* CS2 */
}


void CirrusClockSelect(int freq, bool islaguna)
{
#if 0       /* obsoleted by XFree 3.2 Cirrus clockchip code */
  /* a problem with the Cirrus Clockchip code from XFREE causes 58 MHz to be extremely unstable */
  if ((freq>57900) && (freq<58100)) freq = 57900;
#endif
  /* set DRAM speed (MCLK) = (14318 * val / 8) */
  if (OFLG_ISSET(OPT_XFAST_DRAM)) Outb_SEQ(0x1f,0x25);   /* 66.2 MHz */
  if (OFLG_ISSET(OPT_FAST_DRAM))  Outb_SEQ(0x1f,0x22);   /* 60.8 MHz */
  if (OFLG_ISSET(OPT_MED_DRAM))   Outb_SEQ(0x1f,0x1f);   /* 55.5 MHz */
  if (OFLG_ISSET(OPT_SLOW_DRAM))  Outb_SEQ(0x1f,0x1c);   /* 50.1 MHz */

  SET_CLOCKBITS_0_1(3);
  CirrusSetClock(freq, islaguna);  /* must be AFTER MCLK setting, since CirrusSetClock() uses MCLK */
}



void Video7ClockSelect(int no)
{
  if (no < 2) SET_CLOCKBITS_0_1(no);
  else SET_CLOCKBITS_0_1(2);
  outw(0xA4 | (no << 10), 0x3C4);
}


void ALIClockSelect(int chipset, int no)
{
#if 0
  /* original XFREE code */
  int temp = Inb_GR_CTL(0x0B);
  (no <= 7) ? Outb_GR_CTL(0x0B, temp | 0x01) : Outb_GR_CTL(0x0B, temp & 0xFC);
  SET_CLOCKBITS_0_1(no);
  temp = Inb_GR_CTL(0x0C);
  Outb_GR_CTL(0x0C, ((no & 0x04) << 3));
#else
  /* cleaned up. OK ??? */
  if (chipset!=CS_AL2101)
  {
    /* division by 2 for low clocks */
    Outbit_GR_CTL(0x0B, 0, (no <= 7) ? 1 : 0);
    Outbit_GR_CTL(0x0B, 1, 0);
  }
  SET_CLOCKBITS_0_1(no);                       /* clock bits 0 and 1 */
  Outbit_GR_CTL(0x0C, 5, no & 0x04);           /* clock bit 2 */
#endif
  /* is this REALLY necessary? could someone please try this out ? */
  Outbit_CRTC(0x1A, 4, 0);   /* re-lock ALI ext regs --- This is NOT the good place for this */
}


void OAKClockSelect(int chipset, int no)
{
  if (chipset==CS_OTI87)
    {
      Outb_OTI(OTI87_CLOCK, no);
    }
  else
    {
      SET_CLOCKBITS_0_1(no);
      Outbit_OTI(OTI_MISC, 5, no & 0x04);
    }
 }

void SISClockSelect(int no)
{
  /*
   * Do CS0 and CS1 and set them - makes index 7 valid
   */
  SET_CLOCKBITS_0_1(3);

/*  Outb_SEQ(0x07, (Inb_SEQ(0x07) & 0xF0) | no);*/
  Outb_SEQ(0x07, no);
}

void RealTekClockSelect(int no)
{
  SET_CLOCKBITS_0_1(no);
  Outbit_GR_CTL(0x0C, 5, no & 0x04); /* CS2 */
  Outb_GR_CTL(0x0B, (no & 0x08) ? 0x04 : 0x02); /* CS3, which is a clock divide-by-2 */
}

void ARKClockSelect(int no)
{
  SET_CLOCKBITS_0_1(no);
  Outb_SEQ( 0x11, (Inb_SEQ(0x11) & 0x3F) | ((no & 0x0C) << 4) ); /* CS2-3 */
}

void NCRClockSelect(int chipset, int no)
{
  SET_CLOCKBITS_0_1(no);
  switch (chipset)
  {
    case CS_NCR22E: 
        Outbit_SEQ(0x1F, 6, (no & 0x04)); /* CS2 */
        break;
    case CS_NCR32:
        Outbit_SEQ(0x1F, 5, (no & 0x04)); /* CS2 */
        Outbit_SEQ(0x1F, 6, (no & 0x08)); /* CS3 */
        break;
    default: PERROR(("Illegal NCR chipset\n"));
  }
}

void GVGAClockSelect(int no)
{
  SET_CLOCKBITS_0_1(no);
  Outbit_SEQ( 0x7, 0, no & 0x04); /* CS2 */
}

void MXClockSelect(int no)
{
  SET_CLOCKBITS_0_1(no);
  Outbit_SEQ( 0xC4, 0, no & 0x04); /* CS2 */
}

/* don't know more about the Matrox Millennium clock bits... */
void MatroxClockSelect(int no)
{
  SET_CLOCKBITS_0_1(no);
}

