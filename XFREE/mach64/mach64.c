/*  SVGATextMode -- An SVGA textmode manipulation/enhancement tool
 *
 *  Copyright (C) 1995,1996,1997  Koen Gadeyne
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
 *** mach64.c, mode validation functions for mach64 cards
 *** Contributed by M. Grimrath (m.grimrath@tu-bs.de)
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#define STM_XFREE 1     /* avoids redefinition of SCREEN_ON */
#include "messages.h"
#include "chipset.h"
#include "Xmd.h"
#include "../common_hw/xf86_PCI.h"
#include "mach64_mem_access.h"
#include "mach64.h"
#include "regmach64.h"

#define VGA_MISC_R 0x3CC

/* Variables ************************************************************/
static unsigned ioCONFIG_CHIP_ID;
static unsigned ioCONFIG_CNTL;
static unsigned ioSCRATCH_REG0;
static unsigned ioSCRATCH_REG1;
static unsigned ioCONFIG_STAT0;
static unsigned ioMEM_CNTL;
static unsigned ioDAC_REGS;
static unsigned ioDAC_CNTL;
static unsigned ioGEN_TEST_CNTL;
static unsigned ioCLOCK_CNTL;   
static unsigned ioCRTC_GEN_CNTL;

static unsigned mach64ChipType;
static unsigned char mach64CXClk;
static unsigned mach64ChipRev;

static struct {   /* Information about clock chip */
  unsigned char ClockType;
  unsigned char pad0;
  unsigned short MinFreq;
  unsigned short MaxFreq;
  unsigned char CXClk;
  unsigned char pad1;
  unsigned short RefFreq;
  unsigned short RefDivider;
  unsigned short NAdj;
  unsigned short DRAMMemClk;
  unsigned short VRAMMemClk;
} info;

int ATIDivide(int Numerator, int Denominator, int Shift, const int RoundingKind);
    

/************************************************************************/
/* Most code in here is (C) XFree86, Inc.                               */
static void
mach64PrintCTPLL()
{
#if 0
  int i;
  unsigned char pll[16];
  int R = 1432;
  int M, N, P;
  
  for (i = 0; i < 16; i++) {
    outb(ioCLOCK_CNTL + 1, i << 2);
    PDEBUG(("PLL register %2d: 0x%02x\n", i, pll[i] = inb(ioCLOCK_CNTL + 2)));
  }
  M = pll[PLL_REF_DIV];

  N = pll[VCLK0_FB_DIV];
  P = 1 << (pll[VCLK_POST_DIV] & VCLK0_POST);

  PDEBUG(("VCLK0: M=%d, N=%d, P=%d, Clk=%.2f\n", M, N, P,
	 (double)((2 * R * N)/(M * P)) / 100.0));
  N = pll[VCLK1_FB_DIV];
  P = 1 << ((pll[VCLK_POST_DIV] & VCLK1_POST) >> 2);

  PDEBUG(("VCLK1: M=%d, N=%d, P=%d, Clk=%.2f\n", M, N, P,
	 (double)((2 * R * N)/(M * P)) / 100.0));
  N = pll[VCLK2_FB_DIV];
  P = 1 << ((pll[VCLK_POST_DIV] & VCLK2_POST) >> 4);

  PDEBUG(("VCLK2: M=%d, N=%d, P=%d, Clk=%.2f\n", M, N, P,
	 (double)((2 * R * N)/(M * P)) / 100.0));
  N = pll[VCLK3_FB_DIV];
  P = 1 << ((pll[VCLK_POST_DIV] & VCLK3_POST) >> 6);

  PDEBUG(("VCLK3: M=%d, N=%d, P=%d, Clk=%.2f\n", M, N, P,
	 (double)((2 * R * N)/(M * P)) / 100.0));
  N = pll[MCLK_FB_DIV];
  P = 2;
  PDEBUG(("MCLK:  M=%d, N=%d, P=%d, Clk=%.2f\n", M, N, P,
	 (double)((2 * R * N)/(M * P)) / 100.0));
#endif
}

static unsigned mach64IOBase = 0;
static Bool ACertainSubsetOfATIChips;

/************************************************************************/
/* Some code in here is (C) XFree86, Inc.                               */
void Mach64_verify_and_init(void)
{ int memdes;
  pciConfigPtr pcrp, *pcrpp;
  int BlockIO = 1;
  int found_clock   = CLKCHIP_NONE;
   unsigned found_chiptype = 0;
   int found_clock_rev = 0;
  int found_chipset = CS_NONE;
  char signature[10];
   int Mach64=1;
  unsigned long tmp;
  const unsigned long bios_base = 0xC0000;    
  
  
  /* Need access to BIOS ROM */
  if ((memdes = memacc_open()) < 0) {
    /* Actually cannot fail, since we already have IO port access */
    PERROR(("Cannot read BIOS ROM.\n"
	    "You must be superuser, or the program must be setuid root!\n"));
    return;
  }
  

  PDEBUG(("Mach64: Probing for PCI card\n"));
  pcrpp = xf86scanpci(0);
  if (pcrpp == NULL) {
    /* No PCI, up to now this is equal to 'not found' */
    PDEBUG(("No PCI found\n"));
    goto exit_mem;
  } else {
    /* Found PCI device(s), search for ATI */
    for (; *pcrpp != NULL; pcrpp++) {
      pcrp = *pcrpp;
      if (pcrp->_vendor == PCI_ATI_VENDOR_ID) {
	 PDEBUG(("Mach64: device %x, rev_id %x\n",pcrp->_device, pcrp->_rev_id));
	 switch(pcrp->_device)
	   {
	    case PCI_MACH64_CT_ID:
	    case PCI_MACH64_ET_ID:
	    case PCI_MACH64_VT_ID:
	    case PCI_MACH64_VU_ID:
	    case PCI_MACH64_GT_ID:
	    case PCI_MACH64_GU_ID:
	    case PCI_MACH64_GB_ID:
	    case PCI_MACH64_GD_ID:
	    case PCI_MACH64_GI_ID:
	    case PCI_MACH64_GP_ID:
	    case PCI_MACH64_GQ_ID:
	      found_chiptype = pcrp->_device;
	      found_clock_rev = pcrp->_rev_id;
	      found_clock = CLKCHIP_MACH64;
	   }
	/*
	 * The docs say check (pcrp->_user_config_0 & 0x04) 
	 * for BlockIO but this doesn't seem to be reliable.
	 * Instead check if pcrp->_base1 is non-zero.
	 */
	if (pcrp->_base1 & 0xfffffffc) {
	  BlockIO = 1;
	  mach64IOBase = pcrp->_base1 & (pcrp->_base1 & 0x1 ?
				   0xfffffffc : 0xfffffff0);
	  /* If the Block I/O bit isn't set in userconfig, set it */
	  PDEBUG(("Mach64: PCI userconfig0 = 0x%02x\n", pcrp->_user_config_0));
	  if ((pcrp->_user_config_0 & 0x04) != 0x04) {
	    PDEBUG(("Mach64: Setting bit 0x04 in PCI userconfig for card %ld\n",
		    pcrp->_cardnum));
	    xf86writepci(0, pcrp->_bus,
			 pcrp->_cardnum, pcrp->_func,
			 PCI_REG_USERCONFIG, 0x04, 0x04);
	  }
	} else {
	  BlockIO = 0;
	  switch (pcrp->_user_config_0 & 0x03) {
	    case 0:  mach64IOBase = 0x2EC; break;
	    case 1:  mach64IOBase = 0x1CC; break;
	    case 2:  mach64IOBase = 0x1C8; break;
	    default: 
	    PDEBUG(("Mach64: Cannot get mach64IOBase\n"));
	    goto exit_pci;
	  }
	} /* if */
      } /* if */
    } /* for */
  } /* if PCI */
   if (found_clock != CLKCHIP_NONE) {
      PDEBUG(("Mach64: use %s I/O @ 0x%04X\n",(BlockIO ? "block" : "sparse"),mach64IOBase));
      if (BlockIO) {
	 ioCONFIG_CHIP_ID = mach64IOBase + CONFIG_CHIP_ID;
	 ioCONFIG_CNTL    = mach64IOBase + CONFIG_CNTL;
	 ioSCRATCH_REG0   = mach64IOBase + SCRATCH_REG0;
	 ioSCRATCH_REG1   = mach64IOBase + SCRATCH_REG1;
	 ioCONFIG_STAT0   = mach64IOBase + CONFIG_STAT0;
	 ioMEM_CNTL       = mach64IOBase + MEM_CNTL;
	 /*ioDAC_REGS       = mach64IOBase + DAC_REGS;*/
	 /*ioDAC_CNTL       = mach64IOBase + DAC_CNTL;*/
	 ioGEN_TEST_CNTL  = mach64IOBase + GEN_TEST_CNTL;
	 ioCLOCK_CNTL     = mach64IOBase + CLOCK_CNTL;
	 ioCRTC_GEN_CNTL  = mach64IOBase + CRTC_GEN_CNTL;
      } else {
	 ioCONFIG_CHIP_ID = mach64IOBase + (sioCONFIG_CHIP_ID << 10);
	 ioCONFIG_CNTL    = mach64IOBase + (sioCONFIG_CNTL << 10);
	 ioSCRATCH_REG0   = mach64IOBase + (sioSCRATCH_REG0 << 10);
	 ioSCRATCH_REG1   = mach64IOBase + (sioSCRATCH_REG1 << 10);
	 ioCONFIG_STAT0   = mach64IOBase + (sioCONFIG_STAT0 << 10);
	 ioMEM_CNTL       = mach64IOBase + (sioMEM_CNTL << 10);
	 ioDAC_REGS       = mach64IOBase + (sioDAC_REGS << 10);
	 ioDAC_CNTL       = mach64IOBase + (sioDAC_CNTL << 10);
	 ioGEN_TEST_CNTL  = mach64IOBase + (sioGEN_TEST_CNTL << 10);
	 ioCLOCK_CNTL     = mach64IOBase + (sioCLOCK_CNTL << 10);
	 ioCRTC_GEN_CNTL  = mach64IOBase + (sioCRTC_GEN_CNTL << 10);
      }
   } else {
      PWARNING(("Mach64: Cannot find ATI PCI videocard\n"));
   }
   

  PDEBUG(("Mach64: Probing for ATI BIOS\n"));
  memacc_fetch(memdes, bios_base + 0x30, signature, 10);
  if (memcmp(signature, " 761295520", 10)) {
    PWARNING(("Mach64: no ATI BIOS found\n"));
    goto exit_pci;
  }
	
  
  PDEBUG(("Mach64: Probing for MACH64 SCRATCH_REG0\n"));
  tmp = inl(ioSCRATCH_REG0);
  outl(ioSCRATCH_REG0 ,0x55555555);
   if (inl(ioSCRATCH_REG0) != 0x55555555) {
      PWARNING(("Mach64: Probe failed on read 1\n"));
      Mach64=0;
   } else {
      outl(ioSCRATCH_REG0, 0xaaaaaaaa);
      if (inl(ioSCRATCH_REG0) != 0xaaaaaaaa) {
	 PWARNING(("Mach64: Probe failed an read 2\n"));
	 Mach64=0;
      }
   }
   outl(ioSCRATCH_REG0, tmp);
   if (Mach64 == 0) goto exit_pci;
   found_chipset = CS_ATIMACH64;


  PDEBUG(("Mach64: Probing for Mach64 CT (redundant if PCI)\n"));
  tmp = inl(ioCONFIG_CHIP_ID);
  switch(tmp & CFG_CHIP_TYPE) {
   case MACH64_CT_ID:
   case MACH64_ET_ID:
   case MACH64_VT_ID:
   case MACH64_VU_ID:
   case MACH64_GT_ID:
   case MACH64_GU_ID:
   case MACH64_GB_ID:
   case MACH64_GD_ID:
   case MACH64_GI_ID:
   case MACH64_GP_ID:
   case MACH64_GQ_ID:
     if (found_chiptype != (tmp & CFG_CHIP_TYPE)) {
	PWARNING(("Mach64: PCI and card disagree on clockchip type\n"
		  "choosing card one.\n"));
     }
     found_chiptype = (tmp & CFG_CHIP_TYPE);
     found_clock = CLKCHIP_MACH64;
  }
  
  /* Write found values if not overwritten in config file */
  if (chipset == CS_NONE) {
    PDEBUG(("Mach64: auto select for chipset %s\n",ChipsetRec[chipset].name_str));
    chipset = found_chipset;
  }
  if (clock_data.clockchiptype == CLKCHIP_NONE) {
    PDEBUG(("Mach64: auto select for clockchip %s\n",
	    ClockchipRec[clock_data.clockchiptype].name_str));
    clock_data.clockchiptype = found_clock;
  }
  

  /* Read out clock chip timings from BIOS */
  { unsigned short rom_table;
    unsigned short freq_table;

    PDEBUG(("Getting frequency values from BIOS ROM\n"));
    memacc_fetch(memdes, bios_base + 0x48, &rom_table, 2);
    memacc_fetch(memdes, bios_base + rom_table + 8*2, &freq_table, 2);
    memacc_fetch(memdes, bios_base + freq_table, &info, sizeof(info));
    clock_data.refclk   = info.RefFreq*10;
    if (clock_data.maxclock > info.MaxFreq*10) {
      /* there's no real point in this, since validation has already been done... */
      clock_data.maxclock = info.MaxFreq*10;
    }
    PDEBUG(("MinFreq: %d; MaxFreq: %d; RefFreq: %d.\n", \
      info.MinFreq*10, info.MaxFreq*10, info.RefFreq*10));
     mach64CXClk = info.CXClk;
    
    if (info.ClockType != CLK_INTERNAL) {
      PWARNING(("Mach64: Error in BIOS? Mach64 w/o internal clock\n"));
    }
    mach64PrintCTPLL();
  }
  
  
  /* Check here if found chipset/clock match with config file */
  /* print out warning if not                                 */
  if ((found_chipset != CS_NONE) && (found_chipset != chipset)) {
    PWARNING(("Mach64: Didn't find %s chipset\n",ChipsetRec[chipset].name_str));
  }
  if ((found_clock != CLKCHIP_NONE) && (found_clock != clock_data.clockchiptype)) {
    PWARNING(("Mach64: Didn't find %s clockchip\n",
	      ClockchipRec[clock_data.clockchiptype].name_str));
  }
  
   mach64ChipType = found_chiptype;
   mach64ChipRev = found_clock_rev;

   ACertainSubsetOfATIChips =
  (((mach64ChipType == MACH64_VT_ID || mach64ChipType == MACH64_GT_ID) &&
      (mach64ChipRev & 0x07)) ||
      mach64ChipType == MACH64_VU_ID ||
      mach64ChipType == MACH64_GU_ID ||
      mach64ChipType == MACH64_GB_ID ||
      mach64ChipType == MACH64_GD_ID ||
      mach64ChipType == MACH64_GI_ID ||
      mach64ChipType == MACH64_GP_ID ||
      mach64ChipType == MACH64_GQ_ID);

  exit_pci: xf86cleanpci();
  exit_mem: memacc_close(memdes);
}


/*
 * NOTE: This will not work reliably for all clock values. It only works in
 * a small range near the original textmode clock (the BIOS sets the DSP
 * correct for that range): the DSP needs to be configured depending on the
 * pixel clock. This is not implemented.
 *
 * See mach64SetDSPRegs() in the XFree86 MACH64 code. This requires lots of
 * memory mapping to take place...
 */



/************************************************************************/
/* Most code in here is (C) XFree86, Inc.                               */
void Mach64_SetClock(int freq, int charwidth)
{ char old_crtc_ext_disp;
  int M, N, P, R;
  float Q;
  int postDiv;
  int mhz100 = freq / 10;
  unsigned char tmp1, tmp2;
  int ext_div = 0;
  int clkCntl;

/* Various memory-related things needed to set DSP registers */
  int ATIXCLKFeedbackDivider,
           ATIXCLKReferenceDivider,
           ATIXCLKPostDivider;
  CARD16 ATIXCLKMaxRASDelay,
              ATIXCLKPageFaultDelay,
              ATIDisplayLoopLatency,
              ATIDisplayFIFODepth;
  CARD32 IO_Value;
  int trp;
  CARD8 ATIMemoryType;
  int Multiplier, Divider;
  int dsp_precision, dsp_on, dsp_off, dsp_xclks;
  int tmp, vshift, xshift;
  CARD32 dsp_on_off, dsp_config;

  if (clock_data.clockchiptype != CLKCHIP_MACH64) {
    PERROR(("mach64ct: Sorry, can only handle Mach64 with internal\n"
	    "clockchip so far\n"));
  }

  old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
  outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

  M = info.RefDivider;
  R = info.RefFreq;

  /* Read out which clock for textmode is in use. Usually it is clock #1, */
  /* but you never know what strange configurations exist...              */
  clkCntl = (inb(VGA_MISC_R) >> 2) & 3;
  PDEBUG(("mach64ct: Programming clock #%d, CXClk: %d\n",clkCntl, mach64CXClk));
  
  if (mhz100 < info.MinFreq) mhz100 = info.MinFreq;
  if (mhz100 > info.MaxFreq) mhz100 = info.MaxFreq;

  Q = (mhz100 * M)/(2.0 * R);

  if (ACertainSubsetOfATIChips) {
     if (Q > 255) {
	PWARNING(("mach64ct: Warning: Q > 255\n"));
        Q = 255;
        P = 0;
        postDiv = 1;
     }
     else if (Q > 127.5) {
	P = 0;
	postDiv = 1;
     }
     else if (Q > 85) {
	P = 1;
	postDiv = 2;
     }
     else if (Q > 63.75) {
	P = 0;
	postDiv = 3;
	ext_div=1;
     }
     else if (Q > 42.5) {
	P = 2;
	postDiv = 4;
     }
     else if (Q > 31.875) {
	P = 2;
	postDiv = 6;
	ext_div=1;
     }
     else if (Q > 21.25) {
	P = 3;
	postDiv = 8;
     }
     else if (Q >= 10.6666666667) {
	P = 3;
	postDiv = 12;
	ext_div = 1;
     } else {
	PWARNING(("mach64ct: Warning: Q < 10.6666666667\n"));
	P = 3;
	postDiv = 12;
	ext_div =1;
     } 
  } else {
     if (Q >255) {
	PWARNING(("mach64ct: Warning: Q > 255\n"));
	P = 0;
	Q = 255;
     } else if (Q > 127.5) 
	P = 0;
     else if (Q >63.75)
	 P = 1;
     else if (Q > 31.875)
	 P = 2;
     else if (Q >= 16)
	 P = 3;
     else {
	PWARNING(("mach64ct: Warning: Q < 16\n"));
	Q=16;
	P = 3;
     }
     postDiv = 1 << P;
  }
  N = (int)(Q * postDiv + 0.5);
  
  PDEBUG(("mach64ct: Q = %f N = %d P = %d, postDiv = %d R = %d M = %d\n",
	  Q, N, P, postDiv, R, M));
  PDEBUG(("mach64ct: ext_div = %d\n", ext_div));
  PDEBUG(("mach64ct: New freq: %.2f\n", 
	  (double)((2 * R * N)/(M * postDiv)) / 100.0));

#if 1
  outb(ioCLOCK_CNTL + 1, PLL_VCLK_CNTL << 2);
  tmp1 = inb(ioCLOCK_CNTL + 2);
  outb(ioCLOCK_CNTL + 1, (PLL_VCLK_CNTL  << 2) | PLL_WR_EN);
  outb(ioCLOCK_CNTL + 2, tmp1 | 0x04);
  outb(ioCLOCK_CNTL + 1, (PLL_VCLK_CNTL << 2) | PLL_WR_EN);
  outb(ioCLOCK_CNTL + 2, tmp1 & ~0x04);


  outb(ioCLOCK_CNTL + 1, PLL_VCLK_CNTL << 2);
  PDEBUG(("PLL_VCLK_CNTL = %d\n", inb(ioCLOCK_CNTL + 2)));

  outb(ioCLOCK_CNTL + 1, VCLK_POST_DIV << 2);
  PDEBUG(("VCLK_POST_DIV = %d\n", inb(ioCLOCK_CNTL + 2)));

  outb(ioCLOCK_CNTL + 1, (VCLK0_FB_DIV + clkCntl) << 2);
  PDEBUG(("VCLK0_FB_DIV = %d\n", inb(ioCLOCK_CNTL + 2)));

  if (ACertainSubsetOfATIChips) {
      outb(ioCLOCK_CNTL +1, PLL_XCLK_CNTL << 2);
      PDEBUG(("PLL_XCLK_CNTL = %d\n", inb(ioCLOCK_CNTL + 2)));
  }

  PDEBUG(("DSPCONFIG @0x%x = 0x%08x\n", mach64IOBase + DSP_CONFIG, inl(mach64IOBase + DSP_CONFIG)));

  PDEBUG(("DSPONOFF @0x%x = 0x%08x\n", mach64IOBase + DSP_ON_OFF, inl(mach64IOBase + DSP_ON_OFF)));
  
#endif

  outb(ioCLOCK_CNTL + 1, PLL_VCLK_CNTL << 2);
  tmp1 = inb(ioCLOCK_CNTL + 2);
  outb(ioCLOCK_CNTL + 1, (PLL_VCLK_CNTL  << 2) | PLL_WR_EN);
  outb(ioCLOCK_CNTL + 2, tmp1 | 0x04);

  outb(ioCLOCK_CNTL + 1, VCLK_POST_DIV << 2);
  tmp2 = inb(ioCLOCK_CNTL + 2);
  outb(ioCLOCK_CNTL + 1, ((VCLK0_FB_DIV + clkCntl) << 2) | PLL_WR_EN);
  outb(ioCLOCK_CNTL + 2, N);
  outb(ioCLOCK_CNTL + 1, (VCLK_POST_DIV << 2) | PLL_WR_EN);
  outb(ioCLOCK_CNTL + 2,
       (tmp2 & ~(0x03 << (2 * clkCntl))) | (P << (2 * clkCntl)));

  outb(ioCLOCK_CNTL + 1, (PLL_VCLK_CNTL << 2) | PLL_WR_EN);
  outb(ioCLOCK_CNTL + 2, tmp1 & ~0x04);

  if (ACertainSubsetOfATIChips) {
     outb(ioCLOCK_CNTL +1, PLL_XCLK_CNTL << 2);
     tmp1= inb(ioCLOCK_CNTL + 2);
     outb(ioCLOCK_CNTL + 1, (PLL_XCLK_CNTL << 2) | PLL_WR_EN);
     if (ext_div)
	 outb(ioCLOCK_CNTL + 2, tmp1 | (1 << (clkCntl + 4)));
     else 
	 outb(ioCLOCK_CNTL + 2, tmp1 & ~(1 << (clkCntl + 4)));
  }
  

/* DSP code stolen from XFree86 3.3.2 SVGA ATI driver atidsp.c */

 /* modified ATIDSPProbe() starts here */
 
  /* Retrieve XCLK settings */
  outb(ioCLOCK_CNTL + 1, PLL_XCLK_CNTL << 2);
  IO_Value = inb(ioCLOCK_CNTL + 2);
  ATIXCLKPostDivider = GetBits(IO_Value, PLL_XCLK_SRC_SEL);

  ATIXCLKReferenceDivider = info.RefDivider;

  switch (ATIXCLKPostDivider)
  {
      case 0: case 1: case 2: case 3:
          break;

      case 4:
          ATIXCLKReferenceDivider *= 3;
          ATIXCLKPostDivider = 0;
          break;

      default:
          PWARNING(("Unsupported XCLK source: %d", ATIXCLKPostDivider));
/*          return FALSE;*/
  }

  /* MCLK FB divider is *2 or *4 depending on PLL_MFB_TIMES_4_2B */
  ATIXCLKPostDivider -= GetBits(IO_Value, PLL_MFB_TIMES_4_2B);
  outb(ioCLOCK_CNTL + 1, MCLK_FB_DIV << 2);
  ATIXCLKFeedbackDivider = inb(ioCLOCK_CNTL + 2);

  /* Compute maximum RAS delay and friends */
  IO_Value = inl(ioMEM_CNTL);
  trp = GetBits(IO_Value, CTL_MEM_TRP);
  ATIXCLKPageFaultDelay = GetBits(IO_Value, CTL_MEM_TRCD) +
      GetBits(IO_Value, CTL_MEM_TCRD) + trp + 2;
  ATIXCLKMaxRASDelay = GetBits(IO_Value, CTL_MEM_TRAS) + trp + 2;
  ATIDisplayFIFODepth = 32;

  if (ACertainSubsetOfATIChips)      /* was: if (ATIChip < ATI_CHIP_264GT3) */
  {
      ATIXCLKPageFaultDelay += 2;
      ATIXCLKMaxRASDelay += 3;
      ATIDisplayFIFODepth = 24;
  }
  
  PDEBUG(("ATIDisplayFIFODepth = %d\n", ATIDisplayFIFODepth));
      
  IO_Value = inl(ioCONFIG_STAT0);
  ATIMemoryType = GetBits(IO_Value, CFG_MEM_TYPE);
  PDEBUG(("ATIMemoryType = %d\n", ATIMemoryType));
  switch (ATIMemoryType)
  {
      case MEM_264_DRAM:
#ifdef WHOCARES
          if (ATIvideoRam <= 1024)
              ATIDisplayLoopLatency = 10;
          else
#endif
          {
              ATIDisplayLoopLatency = 8;
              ATIXCLKPageFaultDelay += 2;
          }
          break;

      case MEM_264_EDO:
      case MEM_264_PSEUDO_EDO:
#ifdef WHOCARES
          if (ATIvideoRam <= 1024)
              ATIDisplayLoopLatency = 9;
          else
#endif
          {
              ATIDisplayLoopLatency = 8;
              ATIXCLKPageFaultDelay++;
          }
          break;

      case MEM_264_SDRAM:
#ifdef WHOCARES
          if (ATIvideoRam <= 1024)
              ATIDisplayLoopLatency = 11;
          else
#endif
          {
              ATIDisplayLoopLatency = 10;
              ATIXCLKPageFaultDelay++;
          }
          break;

      case MEM_264_SGRAM:
          ATIDisplayLoopLatency = 8;
          ATIXCLKPageFaultDelay += 3;
          break;

      default:                /* Set maximums */
          ATIDisplayLoopLatency = 11;
          ATIXCLKPageFaultDelay += 3;
          break;
  }

  if (ATIXCLKMaxRASDelay <= ATIXCLKPageFaultDelay)
      ATIXCLKMaxRASDelay = ATIXCLKPageFaultDelay + 1;

 /* modified ATIDSPInit starts here */

#   define Maximum_DSP_PRECISION ((int)GetBits(DSP_PRECISION, DSP_PRECISION))

    /* Compute a memory-to-screen bandwidth ratio */
    PDEBUG(("XCLK = %d kHz\n", 
      ((info.RefFreq*10/ATIXCLKReferenceDivider)*ATIXCLKFeedbackDivider*2) / (1<<ATIXCLKPostDivider) ));
    Multiplier = M * ATIXCLKFeedbackDivider * postDiv;
    Divider = N * ATIXCLKReferenceDivider;
    PDEBUG(("Multiplier = %d ; Divider = %d\n", Multiplier, Divider));
    /* Start by assuming a display FIFO width of 32 bits */
    vshift = (5 - 2) - ATIXCLKPostDivider;

    /* Determine dsp_precision first */
    tmp = ATIDivide(Multiplier * ATIDisplayFIFODepth, Divider, vshift, 1);
    for (dsp_precision = -5;  tmp;  dsp_precision++)
        tmp >>= 1;
    if (dsp_precision < 0)
        dsp_precision = 0;
    else if (dsp_precision > Maximum_DSP_PRECISION)
        dsp_precision = Maximum_DSP_PRECISION;

    xshift = 6 - dsp_precision;
    vshift += xshift;

    PDEBUG(("DSP precision = %d ; vshift = %d ; xshift = %d ; ATIDisplayLoopLatency = %d\n", dsp_precision, vshift, xshift, ATIDisplayLoopLatency));

    /* Move on to dsp_off */
    dsp_off = ATIDivide(Multiplier * (ATIDisplayFIFODepth - 1), Divider,
        vshift, 1);

    /* Next is dsp_on */
    if (dsp_precision < 3)
    {
        /*
         * TODO:  I don't yet know why something like this appears necessary.
         *        But I don't have time to explore this right now.
         */
        dsp_on = ATIDivide(Multiplier * 5, Divider, vshift + 2, -1);
    }
    else
    {
        dsp_on = ATIDivide(Multiplier, Divider, vshift, -1);
        tmp = ATIDivide(ATIXCLKMaxRASDelay, 1, xshift, 1);
        if (dsp_on < tmp)
            dsp_on = tmp;
        dsp_on += tmp + ATIDivide(ATIXCLKPageFaultDelay, 1, xshift, 1);
    }

    dsp_on *= (1<<dsp_precision); /* FIXME : why is this required to get a decent value for DSP_ON ? */

    /* Last but not least:  dsp_xclks */
    /* XCLOCKS/QWORD depends on 9- or 8-pixel font width (of course...) */
    PDEBUG(("charwidth = %d\n", charwidth));
    dsp_xclks = ATIDivide((Multiplier*charwidth)/8, Divider, vshift + 5, 1);
    PDEBUG(("XCLOCKS/QWORD = %d (%1.3f)\n", dsp_xclks, ((float)dsp_xclks)/((float)(1<<(11-dsp_precision)))));

#if 0
    /* the BIOS does this for standard VGA text modes */
    if (freq < 28500) {
      dsp_on_off = 0;
      dsp_config = 0;
    }
#endif

    /* Build DSP register contents */
    dsp_on_off = SetBits(dsp_on, DSP_ON) |
        SetBits(dsp_off, DSP_OFF);
    dsp_config = SetBits(dsp_precision, DSP_PRECISION) |
        SetBits(dsp_xclks, DSP_XCLKS_PER_QW) |
        SetBits(ATIDisplayLoopLatency, DSP_LOOP_LATENCY);
        
  PDEBUG(("dsp_on_off = 0x%08x ; dsp_config = 0x%08x\n", dsp_on_off, dsp_config));
  outl(mach64IOBase + DSP_CONFIG, dsp_config);
  outl(mach64IOBase + DSP_ON_OFF, dsp_on_off);

  usleep(5000);


  (void)inb(ioDAC_REGS); /* Clear DAC Counter */
  outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
}


/*
 * ATIDivide --
 *
 * Using integer arithmetic and avoiding overflows, this function finds the
 * rounded integer that best approximates
 *
 *         Numerator      Shift
 *        ----------- * 2
 *        Denominator
 *
 * using the specified rounding (floor, nearest or ceiling).
 */
int
ATIDivide(int Numerator, int Denominator, int Shift, const int RoundingKind)
{
    int Multiplier, Divider;
    int Rounding = 0;                           /* Default to floor */

    /* Deal with right shifts */
    if (Shift < 0)
    {
        Divider = (Numerator - 1) ^ Numerator;
        Multiplier = 1 << (-Shift);
        if (Divider > Multiplier)
            Divider = Multiplier;
        Numerator /= Divider;
        Denominator *= Multiplier / Divider;
        Shift = 0;
    }

    if (!RoundingKind)                          /* Nearest */
        Rounding = Denominator >> 1;
    else if (RoundingKind > 0)                  /* Ceiling */
        Rounding = Denominator - 1;

    return ((Numerator / Denominator) << Shift) +
            ((((Numerator % Denominator) << Shift) + Rounding) / Denominator);
}
