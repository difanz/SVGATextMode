/*
Some of this code was extracted straight out of XFree86.

This contains several files:
README.RIVA128        - This file
ClockProg.RIVA128.c   - Source for program to get/set the pixel clocks.
clockselect.c         - C file containing all the nessesary code to switch 
                        clocks. Intended to be dropped into SVGATextMode.

This code does set the pixel clock for the RIVA128. It has a programmable
clock that can basically be any requested value to within 1Mhz, most of the 
time within 0.05Mhz of the requested value.

The only problem I've found so far is that it seems to _overwrite_ the
default clocks. ie. If you're in mode 80x25, then setting the pixel clock
with this program will _change_ that clock so that if you try to switch back
to that mode with the VGA clocks, it won't work because the clock has changed.

Other than the clock setting, this video card seems to work just like an 
ET6000. Any code that it uses to set the screen width seems to works fine.

===============================================================================
Here is the bit from nVidia, extracted out XFree86 source:
It contains the source code licence for the extracted code.
*/

 /***************************************************************************\
|*                                                                           *|
|*        Copyright (c) 1996-1998 NVIDIA, Corp.  All rights reserved.        *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.   NVIDIA, Corp. of Sunnyvale, California owns     *|
|*     the copyright  and as design patents  pending  on the design  and     *|
|*     interface  of the NV chips.   Users and possessors of this source     *|
|*     code are hereby granted  a nonexclusive,  royalty-free  copyright     *|
|*     and  design  patent license  to use this code  in individual  and     *|
|*     commercial software.                                                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*     Copyright (c) 1996-1998  NVIDIA, Corp.    NVIDIA  design  patents     *|
|*     pending in the U.S. and foreign countries.                            *|
|*                                                                           *|
|*     NVIDIA, CORP.  MAKES  NO REPRESENTATION ABOUT  THE SUITABILITY OF     *|
|*     THIS SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT     *|
|*     EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORP. DISCLAIMS     *|
|*     ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,  INCLUDING  ALL     *|
|*     IMPLIED   WARRANTIES  OF  MERCHANTABILITY  AND   FITNESS   FOR  A     *|
|*     PARTICULAR  PURPOSE.   IN NO EVENT SHALL NVIDIA, CORP.  BE LIABLE     *|
|*     FOR ANY SPECIAL, INDIRECT, INCIDENTAL,  OR CONSEQUENTIAL DAMAGES,     *|
|*     OR ANY DAMAGES  WHATSOEVER  RESULTING  FROM LOSS OF USE,  DATA OR     *|
|*     PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR OTHER     *|
|*     TORTIOUS ACTION, ARISING OUT  OF OR IN CONNECTION WITH THE USE OR     *|
|*     PERFORMANCE OF THIS SOURCE CODE.                                      *|
|*                                                                           *|
 \***************************************************************************/

/* This code has been updated with stuff from Debian's XFree86 4.3.0,
 * version 4.3.0.dfsg.1-8.
 *
 * Here are the updated notices: */
 /***************************************************************************\
|*                                                                           *|
|*       Copyright 1993-2003 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-2003 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
 \***************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_hw.c,v 1.4 2003/11/03 05:11:25 tsi Exp $ */


#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include "messages.h"
#include "vga_prg.h"

#ifndef DOS

/* PCI stuff -- copied largely from the MGAProbe() code */
#include "include/Xmd.h"
#include "vgaPCI.h"

#define PCI_VENDOR_NVIDIA_SGS	0x12D2

#define UNKNOWN 0
#define NV5  (1<<0)
#define NV10 (1<<1)
#define NV17 (1<<2)
#define NV1X (NV10 | NV17)
#define NV20 (1<<3)
#define NV25 (1<<4)
#define NV2X (NV20 | NV25)
#define NV30 (1<<5)
#define NV31 (1<<6)
#define NV35 (1<<7)
#define NV3X (NV30 | NV31 | NV35)
#define NV40 (1<<8)
#define NV41 (1<<9)
#define NV43 (1<<10)
#define NV44 (1<<11)
#define NV47 (1<<12)
#define NV4X (NV40 | NV41 | NV43 | NV44 | NV47)

unsigned int nvidia_pci_ids[] = {
	0x12D20018,	/*RIVA128*/
	0x10DE0020,	/*RIVATNT*/
	0x10DE0028,	/*RIVATNT2*/
	0x10DE0029,	/*RIVATNT2Ultra*/
	0x10DE002A,	/*UnknownTNT2*/
	0x10DE002B,	/*RIVATNT2*/
	0x10DE002C,	/*Vanta*/
	0x10DE002D,	/*RIVATNT2Model64*/
	0x10DE002E,	/*Vanta*/
	0x10DE002F,	/*Vanta*/
	0x10DE0091,     /*GeForce7800GTX*/
	0x10DE0092,     /*GeForce7800GT*/
	0x10DE0099,     /*GeForceGo7800GTX*/
	0x10DE009D,     /*QuadroFX4500*/
	0x10DE00A0,	/*AladdinTNT2*/
	0x10DE00F0,     /*GeForce6800Ultra*/
	0x10DE00F1,     /*GeForce6600GT*/
        0x10DE00F2,     /*GeForce6600*/
        0x10DE00F3,     /*GeForce6200*/
        0x10DE00F8,     /*Quadro3400*/
        0x10DE00F9,     /*GeForce6800Ultra*/
        0x10DE00FA,     /*GeForcePCX5750*/
        0x10DE00FB,     /*GeForcePCX5900*/
        0x10DE00FC,     /*GeForcePCX5300*/
        0x10DE00FD,     /*QuadroNVS280/FX330*/
        0x10DE00FE,     /*QuadroFX1300*/
        0x10DE00FF,     /*GeForcePCX4300*/
	0x10DE0100,	/*GeForce256*/
	0x10DE0101,	/*GeForceDDR*/
	0x10DE0103,	/*Quadro*/
	0x10DE0110,	/*GeForce2MX/MX400*/
	0x10DE0111,	/*GeForce2MX100/200*/
	0x10DE0112,	/*GeForce2Go*/
	0x10DE0113,	/*Quadro2MXR/EX/Go*/
        0x10DE0140,     /*GeForce6600GT*/
        0x10DE0141,     /*GeForce6600*/
        0x10DE0142,     /*GeForce6600LE*/
        0x10DE0144,     /*GeForceGo6600*/
        0x10DE0145,     /*GeForce6610XL*/
        0x10DE0146,     /*GeForceGo6600TE/6200TE*/
        0x10DE0147,     /*GeForce6700XL*/
        0x10DE0148,     /*GeForce6600*/
        0x10DE0149,     /*GeForce6600GT*/
        0x10DE014E,     /*QuadroFX540*/
        0x10DE014F,     /*GeForce6200*/
        0x10DE0161,     /*GeForce6200TurboCache*/
        0x10DE0162,     /*GeForce6600SETurboCache*/
        0x10DE0164,     /*GeForceGo6200*/
        0x10DE0165,     /*QuadroNVS285*/
        0x10DE0166,     /*GeForceGo6400*/
        0x10DE0167,     /*GeForceGo6200*/
        0x10DE0168,     /*GeForceGo6400*/
        0x10DE0211,     /*GeForce6800*/
        0x10DE0212,     /*GeForce6800LE*/
        0x10DE0215,     /*GeForce6800GT*/
        0x10DE0221,     /*GeForce6200*/
	0x10DE01A0,	/*GeForce2IntegratedGPU*/
	0x10DE0150,	/*GeForce2GTS*/
	0x10DE0151,	/*GeForce2Ti*/
	0x10DE0152,	/*GeForce2Ultra*/
	0x10DE0153,	/*Quadro2Pro*/
	0x10DE0170,	/*GeForce4MX460*/
	0x10DE0171,	/*GeForce4MX440*/
	0x10DE0172,	/*GeForce4MX420*/
	0x10DE0173,	/*GeForce4MX440-SE*/
	0x10DE0174,	/*GeForce4440Go*/
	0x10DE0175,	/*GeForce4420Go*/
	0x10DE0176,	/*GeForce4420Go32M*/
	0x10DE0177,	/*GeForce4460Go*/
	0x10DE0179,	/*GeForce4440Go64M*/
	0x10DE017D,	/*GeForce4410Go16M*/
	0x10DE017C,	/*Quadro4500GoGL*/
	0x10DE0178,	/*Quadro4550XGL*/
	0x10DE017A,	/*Quadro4NVS*/
	0x10DE0181,	/*GeForce4MX440withAGP8X*/
	0x10DE0182,	/*GeForce4MX440SEwithAGP8X*/
	0x10DE0183,	/*GeForce4MX420withAGP8X*/
	0x10DE0186,	/*GeForce4448Go*/
	0x10DE0187,	/*GeForce4488Go*/
	0x10DE0188,	/*Quadro4580XGL*/
	0x10DE0189,	/*GeForce4MXwithAGP8X(Mac)*/
	0x10DE018A,	/*Quadro4280NVS*/
	0x10DE018B,	/*Quadro4380XGL*/
	0x10DE01F0,	/*GeForce4MXIntegratedGPU*/
	0x10DE0200,	/*GeForce3*/
	0x10DE0201,	/*GeForce3Ti200*/
	0x10DE0202,	/*GeForce3Ti500*/
	0x10DE0203,	/*QuadroDCC*/
	0x10DE0250,	/*GeForce4Ti4600*/
	0x10DE0251,	/*GeForce4Ti4400*/
	0x10DE0252,	/*0x0252*/
	0x10DE0253,	/*GeForce4Ti4200*/
	0x10DE0258,	/*Quadro4900XGL*/
	0x10DE0259,	/*Quadro4750XGL*/
	0x10DE025B,	/*Quadro4700XGL*/
	0x10DE0280,	/*GeForce4Ti4800*/
	0x10DE0281,	/*GeForce4Ti4200withAGP8X*/
	0x10DE0282,	/*GeForce4Ti4800SE*/
	0x10DE0286,	/*GeForce44200Go*/
	0x10DE028C,	/*Quadro4700GoGL*/
	0x10DE0288,	/*Quadro4980XGL*/
	0x10DE0289,	/*Quadro4780XGL*/
	0x10DE0301,	/*GeForceFX5800Ultra*/
	0x10DE0302,	/*GeForceFX5800*/
	0x10DE0308,	/*QuadroFX2000*/
	0x10DE0309,	/*QuadroFX1000*/
	0x10DE0311,	/*GeForceFX5600Ultra*/
	0x10DE0312,	/*GeForceFX5600*/
	0x10DE0313,	/*0x0313*/
	0x10DE0314,	/*GeForceFX5600SE*/
	0x10DE0316,	/*0x0316*/
	0x10DE0317,	/*0x0317*/
	0x10DE031A,	/*GeForceFXGo5600*/
	0x10DE031B,	/*GeForceFXGo5650*/
	0x10DE031C,	/*QuadroFXGo700*/
	0x10DE031D,	/*0x031D*/
	0x10DE031E,	/*0x031E*/
	0x10DE031F,	/*0x031F*/
	0x10DE0321,	/*GeForceFX5200Ultra*/
	0x10DE0322,	/*GeForceFX5200*/
	0x10DE0323,	/*GeForceFX5200SE*/
	0x10DE0324,	/*GeForceFXGo5200*/
	0x10DE0325,	/*GeForceFXGo5250*/
	0x10DE0328,	/*GeForceFXGo520032M/64M*/
	0x10DE0329,	/*GeForceFX5200(Mac)*/
	0x10DE032A,	/*QuadroNVS280PCI*/
	0x10DE032B,	/*QuadroFX500*/
	0x10DE032C,	/*GeForceFXGo5300*/
	0x10DE032D,	/*GeForceFXGo5100*/
	0x10DE032F,	/*0x032F*/
	0x10DE0330,	/*GeForceFX5900Ultra*/
	0x10DE0331,	/*GeForceFX5900*/
	0x10DE0332,	/*GeForceFX5900XT*/
	0x10DE0333,	/*GeForceFX5950Ultra*/
	0x10DE0334,	/*0x0334*/
	0x10DE0338,	/*QuadroFX3000*/
	0x10DE0341,	/*GeForceFX5700Ultra*/
	0x10DE0342,	/*GeForceFX5700*/
	0x10DE0343,	/*0x0343*/
	0x10DE0347,	/*0x0347*/
	0x10DE0348,	/*0x0348*/
	0x10DE0349,	/*0x0349*/
	0x10DE034B,	/*0x034B*/
	0x10DE034C,	/*0x034C*/
	0x10DE034E,	/*QuadroFX1100*/
	0x10DE034F,	/*0x034F*/
	0        /* NULL terminator */
};

vgaPCIInformation *vgaPCIInfo;

//=== Constant section ===
//============ Most of these constants come from the NV3 Xserver (XF86_SVGA) =====================
#define PRAMDAC_BASE       0x00680000
#define PRAMDAC_PLL_COEFF  0x00000508
#define PRAMDAC_PLL_COEFF_SELECT   0x0000050C
#define PRAMDAC_PLL2_COEFF 0x00000578
#define PEXTDEV_BASE	   0x00101000

#define NV3_MIN_CLOCK_IN_KHZ  25000    // Not sure about this, but it seems reasonable
#define NV3_MAX_CLOCK_IN_KHZ 230000
#define NV4_MAX_CLOCK_IN_KHZ 350000
#define NV31_34_MAX_CLOCK_IN_KHZ 400000

static int max_clock, is_nv3, pll_coeff, CrystalFreqKHz, twoStagePLL;
 
#define M_MIN 7
#define M_MAX 13

#define P_MIN 0
#define P_MAX 4

//=== Function section ===
// From xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/riva_hw.c in XFree86 3.3.6
/*
 * Calculate the Video Clock parameters for the PLL.
 */
static int CalcVClock
(
    int           clockIn,
    int           double_scan,
    int          *clockOut,
    int          *mOut,
    int          *nOut,
    int          *pOut/*,
    RIVA_HW_INST *chip*/
)
{
    unsigned lowM, highM, highP;
    unsigned DeltaNew, DeltaOld;
    unsigned VClk, Freq;
    unsigned M, N, P;
    
    DeltaOld = 0xFFFFFFFF;

    VClk     = (unsigned)clockIn;
    if (double_scan)
        VClk *= 2;
    
    if (/*chip->*/CrystalFreqKHz == 14318)
    {
        lowM  = 8;
        highM = 14 - (/*chip->Architecture == NV_ARCH_03*/is_nv3);
    }
    else
    {
        lowM  = 7;
        highM = 13 - (/*chip->Architecture == NV_ARCH_03*/is_nv3);
    }                      

    highP = 4 - (/*chip->Architecture == NV_ARCH_03*/is_nv3);
    for (P = 0; P <= highP; P ++)
    {
        Freq = VClk << P;
        if ((Freq >= 128000) && (Freq <= /*chip->MaxVClockFreqKHz*/max_clock))
        {
            for (M = lowM; M <= highM; M++)
            {
		N    = ((VClk << P) * M) / /*chip->*/CrystalFreqKHz;
		if (N <= 255) {
		    Freq = ((/*chip->*/CrystalFreqKHz * N) / M) >> P;
		    if (Freq > VClk)
			DeltaNew = Freq - VClk;
		    else
			DeltaNew = VClk - Freq;
		    if (DeltaNew < DeltaOld)
		    {
			*mOut     = M;
			*nOut     = N;
			*pOut     = P;
			*clockOut = Freq;
			DeltaOld  = DeltaNew;
		    }
		}
            }
        }
    }
    return (DeltaOld != 0xFFFFFFFF);
}

static int CalcVClock2Stage (
    int           clockIn,
    int           double_scan,
    int          *clockOut,
    int          *mOut,
    int          *nOut,
    int          *pOut,
    unsigned	 *pllBOut
)
{
    unsigned DeltaNew, DeltaOld;
    unsigned VClk, Freq;
    unsigned M, N, P;

    DeltaOld = 0xFFFFFFFF;

    *pllBOut = 0x80000401;  /* fixed at x4 for now */

    VClk = (unsigned)clockIn;
    if (double_scan)
        VClk *= 2;

    for (P = 0; P <= 6; P++) {
        Freq = VClk << P;
        if ((Freq >= 400000) && (Freq <= 1000000)) {
            for (M = 1; M <= 13; M++) {
                N = ((VClk << P) * M) / (CrystalFreqKHz << 2);
                if((N >= 5) && (N <= 255)) {
                    Freq = (((CrystalFreqKHz << 2) * N) / M) >> P;
                    if (Freq > VClk)
                        DeltaNew = Freq - VClk;
                    else
                        DeltaNew = VClk - Freq;
                    if (DeltaNew < DeltaOld) {
			*mOut     = M;
			*nOut     = N;
			*pOut     = P;
                        *clockOut = Freq;
                        DeltaOld  = DeltaNew;
                    }
                }
            }
        }
    }
    return (DeltaOld != 0xFFFFFFFF);
}

// get GPU architecture
short get_gpu_arch(int device_id)
{
    short arch = UNKNOWN;
    switch(device_id & 0xff0)
    {
    case 0x20:
	arch = NV5;
	break;
    case 0x100:
    case 0x110:
    case 0x150:
    case 0x1a0:
	arch = NV10;
	break;
    case 0x170:
    case 0x180:
    case 0x1f0:
	arch = NV17;
	break;
    case 0x200:
	arch = NV20;
	break;
    case 0x250:
    case 0x280:
    case 0x320:	/* We don't treat the FX5200/FX5500 as FX cards */
	arch = NV25;
	break;
    case 0x300:
	arch = NV30;
	break;
    case 0x330:
	arch = NV35; /* Similar to NV30 but fanspeed stuff works differently */
	break;
	/* Give a seperate arch to FX5600/FX5700 cards as they need different code than other FX cards */
    case 0x310:
    case 0x340:
	arch = NV31;
	break;
    case 0x40:
    case 0x120:
    case 0x130:
    case 0x1d0:
    case 0x210:
    case 0x230:
	arch = NV40;
	break;
    case 0xc0:
	arch = NV41;
	break;
    case 0x140:
	arch = NV43; /* Similar to NV40 but with different fanspeed code */
	break;
    case 0x160:
    case 0x220:
	arch = NV44;
	break;
    case 0x90:
	arch = NV47;
	break;
    case 0xf0:
	/* The code above doesn't work for pci-express cards as multiple architectures share one id-range */
	switch(device_id)
	{
	case 0xf0: /* 6800 */
	case 0xf9: /* 6800Ultra */
	    arch = NV40;
	    break;
	case 0xf1: /* 6600/6600GT */
	case 0xf2: /* 6600GT */
	case 0xf3: /* 6200 */
	    arch = NV43;
	    break;
	case 0xfa: /* PCX5700 */
	    arch = NV31;
	    break;
	case 0xf8: /* QuadroFX 3400 */
	case 0xfb: /* PCX5900 */
	    arch = NV35;
	    break;
	case 0xfc: /* PCX5300 */
	case 0xfd: /* Quadro NVS280/FX330, FX5200 based? */
	case 0xff: /* PCX4300 */
	    arch = NV25;
	    break;
	case 0xfe: /* Quadro 1300, has the same id as a FX3000 */
	    arch = NV35;
	    break;
	}
	break;
    default:
	arch = UNKNOWN;
    }
    return arch;
}


// Set the clock to the given speed (in KHz)
Bool RIVA128ClockSelect( int clockspeed )
{
  int *PRAMDAC0;
  int *PEXTDEV;

  int out;
  int m, n, p;
  unsigned pll1, pll2;
  int i = 0;
  pciConfigPtr pcr = NULL;
  int arch = 0;
  int fd;

  vgaPCIInfo = vgaGetPCIInfo();

  if (vgaPCIInfo && vgaPCIInfo->AllCards) {
     	while ((pcr = vgaPCIInfo->AllCards[i++])) {
		int i, supported = 0;
		for (i = 0; nvidia_pci_ids[i]; i++)
			if( nvidia_pci_ids[i] == (pcr->_device | (pcr->_vendor << 16)) )
				supported++;
				
		if (supported) {
			is_nv3 = (pcr->_vendor == PCI_VENDOR_NVIDIA_SGS) ? 1 : 0;
		        if (!is_nv3) arch = get_gpu_arch(pcr->_device);
			break;
  		}
  	}
  }
  else 
  {
    PERROR(("No supported nVidia GPU found in PCI info!\n"));
    return FALSE;
  }

  if ( !pcr )
  {
    PERROR(("No supported nVidia GPU found in PCI info!\n"));
    return FALSE;
  }
  
  PDEBUG(("PCI BASE0 = 0x%x\n", pcr->_base0));

  // Uses memory mapped registers, so must map memory
  fd = open( "/dev/mem", O_RDWR );
  
  if( fd == -1 )
  {
    PERROR(( "Error opening /dev/mem" ));
    return FALSE;
  }
    
  // mmap the programmable RAMDAC into our address space
  PRAMDAC0 = (int*)mmap( 0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)(pcr->_base0) + PRAMDAC_BASE );

  if( PRAMDAC0 == (int*)-1 )
  {
    PERROR(( "Error mmap'ing /dev/mem" ));
    return FALSE;
  }
    
  // mmap the PEXTDEV into our address space
  PEXTDEV = (int*)mmap( 0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)(pcr->_base0) + PEXTDEV_BASE );

  if( PEXTDEV == (int*)-1 )
  {
    PERROR(( "Error mmap'ing /dev/mem" ));
    return FALSE;
  }

  close( fd );

  // Get chip config
  if (is_nv3) {
	pll_coeff = 0x00010100;
  	max_clock = NV3_MAX_CLOCK_IN_KHZ;
	CrystalFreqKHz = (PEXTDEV[0x00000000/4] & 0x00000020) ? 14318 : 13500;
	twoStagePLL = 0;
  } 
  else
  {
	pll_coeff = 0x00000700;
	CrystalFreqKHz = (PEXTDEV[0x0000/4] & (1 << 6)) ? 14318 : 13500;		
	if(arch & (NV17 | NV2X | NV3X | NV4X))
	{
	   if(PEXTDEV[0x0000/4] & (1 << 22)) CrystalFreqKHz = 27000;
	}
	twoStagePLL = (arch & (NV31 | NV35 | NV4X));

  	max_clock = twoStagePLL ? NV31_34_MAX_CLOCK_IN_KHZ
			        : NV4_MAX_CLOCK_IN_KHZ;
  }

  // Calculate the clock  
  if (twoStagePLL) {
	CalcVClock2Stage ((float) clockspeed, 0, &out, &m, &n, &p, &pll2);
  	pll1 = (m) + (n<<8) + (p<<16);
  	PDEBUG(( "Wanted %dkHz, got %dkHz (m=%d, n=%d, p=%d, pll1=0x%08X, pll2=0x%08X)\n",
	    clockspeed, (int)out, m, n, p, pll1, pll2 ));
  } else {
  	CalcVClock ((float) clockspeed, 0, &out, &m, &n, &p);
  	pll1 = (m) + (n<<8) + (p<<16);
  	PDEBUG(( "Wanted %dkHz, got %dkHz (m=%d, n=%d, p=%d, pll=0x%08X)\n",
	    clockspeed, (int)out, m, n, p, pll1 ));
  }
  
  // Default value is 0x00000100 (NV3)
  // X uses 0x10010100 (NV3) or 0x10000700 (NV4/10/20/30)
  // We use 0x00010100 (NV3) or 0x00000700 (NV4/10/20/30)
  PRAMDAC0[PRAMDAC_PLL_COEFF_SELECT/4] = pll_coeff;  // could use |=

  // Divide by 4 because we're dealing with integers
  PRAMDAC0[PRAMDAC_PLL_COEFF/4] = pll1;
  if (twoStagePLL) PRAMDAC0[PRAMDAC_PLL2_COEFF/4] = pll2;
  
  // Unmap memory
  munmap( PRAMDAC0, 0x1000 );
  munmap( PEXTDEV,  0x1000 );

  // All done
  return TRUE;
}

#else /* DOS */

Bool RIVA128ClockSelect( int clockspeed )
{
  /* Really not possible in DOS? The mmap-stuff can be ported to DOS like in mga_clock.c
   * Just need someone to test it. - Wolfram, Jan 06
   */
  PERROR(("Riva128 chipsets not (yet) supported in DOS! See INSTALL.TXT.\n"));
  return FALSE;
}

#endif
