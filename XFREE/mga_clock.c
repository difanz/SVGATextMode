
/*
 * mga_clock.c
 *
 * Snatched from the XFree86 (3.2Ag) code (vga256/drivers/mga/mgadriver.c)
 */

/* some functions or variables are not used here (because we don't use MMIO),
 * but we need to shut up the compiler
 */

typedef unsigned char uchar;

#include "misc.h"
#define STM_XFREE 1 /* avoid redefinition of SCREEN_ON */
#include "messages.h"

static uchar *MGAMMIOBase = NULL;

#define OUTREG8(x,y)	*(MGAMMIOBase + x) = y
#define INREG8(x)	*(MGAMMIOBase + x)
#define STORM_OPTION 0x40
#define set_pci(r, s, v)	pcibusWrite(MGAPciTag, r, v)
#define get_pci(r, s)		pcibusRead(MGAPciTag, r)

#include "compiler.h"
#include "mgareg.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifdef DOS
#include <dpmi.h>
#endif

/* PCI stuff -- copied largely from the MGAProbe() code */
#include "include/Xmd.h"
#include "vgaPCI.h"

static pciTagRec MGAPciTag;
vgaPCIInformation *vgaPCIInfo;

Bool mga_get_pci_info()
{
	int i = 0;
	pciConfigPtr pcr = NULL;
	int devmem;
	uchar *base;

	if (MGAMMIOBase != NULL) return TRUE;

	vgaPCIInfo = vgaGetPCIInfo();

	if (vgaPCIInfo && vgaPCIInfo->AllCards) {
    	while ((pcr = vgaPCIInfo->AllCards[i++])) {
			if (pcr->_vendor == PCI_VENDOR_MATROX)
			{
				if ((pcr->_device == PCI_CHIP_MGA2064)
				    || (pcr->_device == PCI_CHIP_MGA1064)
				    || (pcr->_device == PCI_CHIP_MGA2164)
				    || (pcr->_device == PCI_CHIP_MGAG200)
				    || (pcr->_device == PCI_CHIP_MGAG400)
				    || (pcr->_device == PCI_CHIP_MGAG100)
				    || (pcr->_device == PCI_CHIP_MGAG200PCI)
				    || (pcr->_device == PCI_CHIP_MGAG100PCI)
				    || (pcr->_device == PCI_CHIP_MGA2164AGP))
					break;
			}
		}
   	} else return(FALSE);

	if (!pcr) return(FALSE);

	if ((pcr->_device != PCI_CHIP_MGA2064)
	    && (pcr->_device != PCI_CHIP_MGA1064)
	    && (pcr->_device != PCI_CHIP_MGA2164)
	    && (pcr->_device != PCI_CHIP_MGAG100)
	    && (pcr->_device != PCI_CHIP_MGAG200)
	    && (pcr->_device != PCI_CHIP_MGAG400)
	    && (pcr->_device != PCI_CHIP_MGAG100PCI)
	    && (pcr->_device != PCI_CHIP_MGAG200PCI)
	    && (pcr->_device != PCI_CHIP_MGA2164AGP))
		return(FALSE);

	/*
	*      OK. It's MGA Millennium I or II or Mystique (or something pretty close)
	*/

	MGAPciTag = pcibusTag(pcr->_bus, pcr->_cardnum, pcr->_func);
	PDEBUG(("MGAPciTag from bus %d, device %d, function %d\n",
		pcr->_bus, pcr->_cardnum, pcr->_func));
	
	/* mmap the control apperature into our address space */
#ifndef DOS
	if ((devmem = open("/dev/mem", O_RDWR)) < 0) {
		PERROR(("Failed to open /dev/mem: %d\n", devmem));
		return FALSE;
	}
        PDEBUG(("MGAbase0: %x ; MGAbase1: %x ; MGAbase2: %x\n",
                 pcr->_base0, pcr->_base1, pcr->_base2));
 
        if (  ((pcr->_device == PCI_CHIP_MGA1064) && (pcr->_rev_id == 0x3))
            || (pcr->_device == PCI_CHIP_MGA2164)
	    || (pcr->_device == PCI_CHIP_MGAG400)
	    || (pcr->_device == PCI_CHIP_MGAG200)
	    || (pcr->_device == PCI_CHIP_MGAG200PCI)
	    || (pcr->_device == PCI_CHIP_MGAG100)
	    || (pcr->_device == PCI_CHIP_MGAG100PCI)
	    || (pcr->_device == PCI_CHIP_MGA2164AGP) )
        {
                /*
                   only for mystique rev 3 with 220MHz RAMDAC
                    mem adress bytes are swapped
                   Also for Millenium II according to XFree86 3.3.1 code
                */ 
/*                PDEBUG(("MGA: swapping mem address bytes to %08lx\n", pcr->_base1));*/
                base = (uchar *)mmap(0, 0x4000, PROT_READ|PROT_WRITE,
                 MAP_SHARED, devmem, (off_t)(pcr->_base1));
        }
        else
        {
                base = (uchar *)mmap(0, 0x4000, PROT_READ|PROT_WRITE,
                MAP_SHARED, devmem, (off_t)(pcr->_base0));
        }
	close(devmem);
	if ((long)base == -1) {
		PERROR(("mmap() failed %d\n", base));
		return FALSE;
	}	
	MGAMMIOBase = base;
#else
      /* Hmmm, shouldn't the swapped address bytes also be covered in DOS?
       * Looks like a bug to me, but can't check it :( - Wolfram, Jan 06
       */ 
      PWARNING(("MGA-Driver might be incomplete in DOS! See INSTALL.TXT.\n"));
      base=(uchar *) malloc(0x4000);
      if (!base || __djgpp_map_physical_memory(base, 0x4000, (off_t)(pcr->_base0)))
      {
              PERROR(("mmap() failed %d\n", base));
              return FALSE;
      }
#endif
	return TRUE;
}

static ulong midCalcFreq(int reff, uchar m, uchar n, uchar p) {
	ulong retval = ((reff * (n + 1)) / (m + 1)) / (p + 1);
	return retval;
}

static ulong midCalcMNPS(ulong freq, uchar *m, uchar *n, uchar *p, uchar *s, int type) {
	ulong Fvco;
	long delta, d;
	uchar nt, mt, pt;
	
	int feed_div_min, feed_div_max;
	int in_div_min, in_div_max;

	int reff;

	switch( type )
	{
		case 0:
			reff         = 14318;
			feed_div_min = 100;
			feed_div_max = 127;
			in_div_min   = 1;
			in_div_max   = 31;
			break;
		default:
			reff         = 27051;
			feed_div_min = 7;
			feed_div_max = 127;
			in_div_min   = 1;
			in_div_max   = 6;
			break;
	}

	// find m and n such that:
	//  'freq' - ((14318 * (n + 1)) / (m + 1)) / (p + 1)
	// is as close to 0 as possible
	// this code performs an exaustive search of the parameter space :-/
	delta = 220000;
	for (pt = 1; pt <= 8; pt <<= 1) {
		Fvco = freq * pt;
		if ((Fvco > 50000) && (Fvco < 220000)) {
			for (nt = feed_div_max; nt >= feed_div_min; nt--) {
				for (mt = in_div_min; mt <= in_div_max; mt++) {
					d = (long)midCalcFreq(reff, mt, nt, (pt - 1)) - (long)freq;
					if (d < 0) d = -d;
					if (d < delta) {
						*n = nt;
						*m = mt;
						*p = pt - 1;
						delta = d;
						*s = 0;
						if (Fvco > 100000) (*s)++;
						if (Fvco > 140000) (*s)++;
						if (Fvco > 180000) (*s)++;
					}
				}
			}
		}
	}
	return midCalcFreq(reff, *m, *n, *p);
}
/* ================== XFREE code starts here ======================== */

void MGAoutTi3026(reg, val)
unsigned char reg, val;
{
	if(MGAMMIOBase)
	{
		OUTREG8(RAMDAC_OFFSET + TVP3026_INDEX, reg);
		OUTREG8(RAMDAC_OFFSET + TVP3026_DATA, val);
	}
	else
	{
		PWARNING(("MGAoutTi3026 BUSTED! Should never get here!\n"));
	}
}

static unsigned char MGAinTi3026(reg)
unsigned char reg;
{
	unsigned char val;
	
	if(MGAMMIOBase)
	{
		OUTREG8(RAMDAC_OFFSET + TVP3026_INDEX, reg);
		val = INREG8(RAMDAC_OFFSET + TVP3026_DATA);
	}
	else
	{
		PWARNING(("MGAinTi3026 BUSTED! Should never get here!\n"));
		val = 0xff;
	}
	return val;
}

void midSetPixClock(ulong freq, int isG2) {
	uchar m, n, p, s;
	uchar tmpByte;
		
	PDEBUG(("Requested PIX clock freq: %d\n", freq));
	freq = midCalcMNPS(freq, &m, &n, &p, &s, isG2);

	PDEBUG(("Actual PIX clock freq: %ld\n", freq));

	// MGA1064SG 5-77
	// Step 1) force screen off
	//setScreenOn(false);
	
	// Step 2) Set pixclkdis to '1'
	MGAoutTi3026(MID_XPIXCLKCTRL, MGAinTi3026(MID_XPIXCLKCTRL) | 0x04);
	
	// Step 3) reprogram pix clock
	// program M, N, P, S
#if DEBUG
	ErrorF("0x%x 0x%x 0x%x\n", MGAinTi3026(MID_XPIXPLLCM), MGAinTi3026(MID_XPIXPLLCN), MGAinTi3026(MID_XPIXPLLCP) );
#endif
	MGAoutTi3026(MID_XPIXPLLCM, m & 0x1f);
	MGAoutTi3026(MID_XPIXPLLCN, n & 0x7f);
	MGAoutTi3026(MID_XPIXPLLCP, ((s << 3) & 0x18) | (p & 0x07));

#if 1
	// Select register set C
	tmpByte = INREG8(VGA_MISC_R);
	tmpByte &= 0xf2; tmpByte |= 0x09;
	OUTREG8(VGA_MISC_W, tmpByte);
#ifdef DEBUG
	PDEBUG(("VGA_MISC_W - wrote 0x%02x, read 0x%02x\n",
		tmpByte, INREG8(VGA_MISC_R)));
#endif
	PDEBUG(("Wait for PLL lock\n"));
	// Step 4) wait for frequency lock
	while ((MGAinTi3026(MID_XPIXPLLSTAT) & 0x40) != 0x40);
#endif

	// Step 5) set pixclkdis to '0'
	MGAoutTi3026(MID_XPIXCLKCTRL, MGAinTi3026(MID_XPIXCLKCTRL) & 0xfb);

	// Step 6) turn screen back on
	//setScreenOn(true);
}


#if 0
void midSetSysClock(ulong freq) {
	uchar m, n, p, s;
	uchar tmpByte;
	ulong tmpUlong;
	
	midCalcMNPS(freq, &m, &n, &p, &s);

	// MGA1064S 5-77
	// Step 1) Set sysclkdis to '1' (disable system clock)
	// 1. Disable system clock
	tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong &= 0xfffffffb;
	tmpUlong |= 0x00000004;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #1 OPTION: 0x%08x\n", tmpUlong));
	
	// Step 2) Select PCI bus clock (sysclksl = '00')
	//tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong &= 0xfffffffc;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #2 OPTION: 0x%08x\n", tmpUlong));

	// Step 3) Set sysclkdis to '0' (enable)
	//tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong &= 0xfffffffb;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #3 OPTION: 0x%08x\n", tmpUlong));
	
	// Step 4) program PLL
	MGAoutTi3026(MID_INDEX, MID_XSYSPLLM);
	MGAoutTi3026(MID_X_DATAREG, m & 0x1f);
	MGAoutTi3026(MID_INDEX, MID_XSYSPLLN);
	MGAoutTi3026(MID_X_DATAREG, n & 0x7f);
	MGAoutTi3026(MID_INDEX, MID_XSYSPLLP);
	MGAoutTi3026(MID_X_DATAREG, ((s << 3) & 0x18) | (p & 0x07));

	// Step 5) wait for lock
	//MGAoutTi3026(MID_INDEX, MID_XSYSPLLSTAT);
	while ((MGAinTi3026(MID_X_DATAREG) & 0x40) != 0x40);

	// Step 5a) set MCLK and GCLK dividers
	//tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong |= 0x018;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #5a OPTION: 0x%08x\n", tmpUlong));
	
	// Step 6) disable clock
	//tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong &= 0xfffffffb;
	tmpUlong |= 0x00000004;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #6 OPTION: 0x%08x\n", tmpUlong));
	
	// Step 7) select sys PLL (sysclksl = '01')
	//tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong |= 0x01;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #7 OPTION: 0x%08x\n", tmpUlong));

	// Step 8) sysclkdis = '0'
	//tmpUlong = get_pci(STORM_OPTION, 4);
	tmpUlong &= 0xfffffffb;
	set_pci(STORM_OPTION, 4, tmpUlong);
	//tmpUlong = get_pci(STORM_OPTION, 4);
	//xprintf(("SysClk #8 OPTION: 0x%08x\n", tmpUlong));
	
}
#endif

/*
 * MGATi3026CalcClock - Calculate the PLL settings (m, n, p).
 *
 * DESCRIPTION
 *   For more information, refer to the Texas Instruments
 *   "TVP3026 Data Manual" (document SLAS098B).
 *     Section 2.4 "PLL Clock Generators"
 *     Appendix A "Frequency Synthesis PLL Register Settings"
 *     Appendix B "PLL Programming Examples"
 *
 * PARAMETERS
 *   f_out		IN	Desired clock frequency.
 *   f_max		IN	Maximum allowed clock frequency.
 *   m			OUT	Value of PLL 'm' register.
 *   n			OUT	Value of PLL 'n' register.
 *   p			OUT	Value of PLL 'p' register.
 *
 * HISTORY
 *   January 11, 1997 - [aem] Andrew E. Mileski
 *   Split off from MGATi3026SetClock.
 */

/* The following values are in kHz */
#define TI_MIN_VCO_FREQ  110000
#define TI_MAX_VCO_FREQ  220000
#define TI_MAX_MCLK_FREQ 100000
#define TI_REF_FREQ      14318.18

static double
MGATi3026CalcClock ( f_out, f_max, m, n, p )
	long f_out;
	long f_max;
	int *m;
	int *n;
	int *p;
{
	int best_m = 0, best_n = 0;
	double f_pll, f_vco;
	double m_err, inc_m, calc_m;

	/* Make sure that f_min <= f_out <= f_max */
	if ( f_out < ( TI_MIN_VCO_FREQ / 8 ))
		f_out = TI_MIN_VCO_FREQ / 8;
	if ( f_out > f_max )
		f_out = f_max;

	/*
	 * f_pll = f_vco / 2 ^ p
	 * Choose p so that TI_MIN_VCO_FREQ <= f_vco <= TI_MAX_VCO_FREQ
	 * Note that since TI_MAX_VCO_FREQ = 2 * TI_MIN_VCO_FREQ
	 * we don't have to bother checking for this maximum limit.
	 */
	f_vco = ( double ) f_out;
	for ( *p = 0; *p < 3 && f_vco < TI_MIN_VCO_FREQ; ( *p )++ )
		f_vco *= 2.0;

	/*
	 * We avoid doing multiplications by ( 65 - n ),
	 * and add an increment instead - this keeps any error small.
	 */
	inc_m = f_vco / ( TI_REF_FREQ * 8.0 );

	/* Initial value of calc_m for the loop */
	calc_m = inc_m + inc_m + inc_m;

	/* Initial amount of error for an integer - impossibly large */
	m_err = 2.0;

	/* Search for the closest INTEGER value of ( 65 - m ) */
	for ( *n = 3; *n <= 25; ( *n )++, calc_m += inc_m ) {

		/* Ignore values of ( 65 - m ) which we can't use */
		if ( calc_m < 3.0 || calc_m > 64.0 )
			continue;

		/*
		 * Pick the closest INTEGER (has smallest fractional part).
		 * The optimizer should clean this up for us.
		 */
		if (( calc_m - ( int ) calc_m ) < m_err ) {
			m_err = calc_m - ( int ) calc_m;
			best_m = ( int ) calc_m;
			best_n = *n;
		}
	}
	
	/* 65 - ( 65 - x ) = x */
	*m = 65 - best_m;
	*n = 65 - best_n;

	/* Now all the calculations can be completed */
	f_vco = 8.0 * TI_REF_FREQ * best_m / best_n;
	f_pll = f_vco / ( 1 << *p );

#ifdef DEBUG
	ErrorF( "f_out=%ld f_pll=%.1f f_vco=%.1f n=%d m=%d p=%d\n",
		f_out, f_pll, f_vco, *n, *m, *p );
#endif

	return f_pll;
}

/*
 * MGATi3026SetMCLK - Set the memory clock (MCLK) PLL.
 *
 * HISTORY
 *   January 11, 1997 - [aem] Andrew E. Mileski
 *   Written and tested.
 */
void
MGATi3026SetMCLK( f_out )
	long f_out;
{
	double f_pll;
	int mclk_m, mclk_n, mclk_p;
	int pclk_m, pclk_n, pclk_p;
	int mclk_ctl, rfhcnt;

	f_pll = MGATi3026CalcClock(
		f_out, TI_MAX_MCLK_FREQ,
		& mclk_m, & mclk_n, & mclk_p
	);

	/* Save PCLK settings */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfc );
	pclk_n = MGAinTi3026( TVP3026_PIX_CLK_DATA );
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfd );
	pclk_m = MGAinTi3026( TVP3026_PIX_CLK_DATA );
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfe );
	pclk_p = MGAinTi3026( TVP3026_PIX_CLK_DATA );
	
	/* Stop PCLK (PLLEN = 0, PCLKEN = 0) */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfe );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, 0x00 );
	
	/* Set PCLK to the new MCLK frequency (PLLEN = 1, PCLKEN = 0 ) */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfc );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, ( mclk_n & 0x3f ) | 0xc0 );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, mclk_m & 0x3f );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, ( mclk_p & 0x03 ) | 0xb0 );
	
	/* Wait for PCLK PLL to lock on frequency */
	while (( MGAinTi3026( TVP3026_PIX_CLK_DATA ) & 0x40 ) == 0 ) {
		;
	}
	
	/* Output PCLK on MCLK pin */
	mclk_ctl = MGAinTi3026( TVP3026_MCLK_CTL );
	MGAoutTi3026( TVP3026_MCLK_CTL, mclk_ctl & 0xe7 ); 
	MGAoutTi3026( TVP3026_MCLK_CTL, ( mclk_ctl & 0xe7 ) | 0x08 );
	
	/* Stop MCLK (PLLEN = 0 ) */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfb );
	MGAoutTi3026( TVP3026_MEM_CLK_DATA, 0x00 );
	
	/* Set MCLK to the new frequency (PLLEN = 1) */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xf3 );
	MGAoutTi3026( TVP3026_MEM_CLK_DATA, ( mclk_n & 0x3f ) | 0xc0 );
	MGAoutTi3026( TVP3026_MEM_CLK_DATA, mclk_m & 0x3f );
	MGAoutTi3026( TVP3026_MEM_CLK_DATA, ( mclk_p & 0x03 ) | 0xb0 );
	
	/* Wait for MCLK PLL to lock on frequency */
	while (( MGAinTi3026( TVP3026_MEM_CLK_DATA ) & 0x40 ) == 0 ) {
		;
	}
	
	/* Set the WRAM refresh divider */
	rfhcnt = ( 332.0 * f_pll / 1280000.0 );
	if ( rfhcnt > 15 )
		rfhcnt = 0;
	pciWriteLong( MGAPciTag, PCI_OPTION_REG, ( rfhcnt << 16 ) |
		( pciReadLong( MGAPciTag, PCI_OPTION_REG ) & ~0xf0000 ));

#ifdef DEBUG
	ErrorF( "rfhcnt=%d\n", rfhcnt );
#endif

	/* Output MCLK PLL on MCLK pin */
	MGAoutTi3026( TVP3026_MCLK_CTL, ( mclk_ctl & 0xe7 ) | 0x10 );
	MGAoutTi3026( TVP3026_MCLK_CTL, ( mclk_ctl & 0xe7 ) | 0x18 );
	
	/* Stop PCLK (PLLEN = 0, PCLKEN = 0 ) */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfe );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, 0x00 );
	
	/* Restore PCLK (PLLEN = ?, PCLKEN = ?) */
	MGAoutTi3026( TVP3026_PLL_ADDR, 0xfc );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, pclk_n );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, pclk_m );
	MGAoutTi3026( TVP3026_PIX_CLK_DATA, pclk_p );
	
	/* Wait for PCLK PLL to lock on frequency */
	while (( MGAinTi3026( TVP3026_PIX_CLK_DATA ) & 0x40 ) == 0 ) {
		;
	}
}

/*
 * MGATi3026SetPCLK - Set the pixel (PCLK) and loop (LCLK) clocks.
 *
 * PARAMETERS
 *   f_pll			IN	Pixel clock PLL frequencly in kHz.
 *   bpp			IN	Bytes per pixel.
 *
 * EXTERNAL REFERENCES
 *   vga256InfoRec.maxClock	IN	Max allowed pixel clock in kHz.
 *
 * HISTORY
 *   January 11, 1997 - [aem] Andrew E. Mileski
 *   Split to simplify code for MCLK (=GCLK) setting.
 *
 *   December 14, 1996 - [aem] Andrew E. Mileski
 *   Fixed loop clock to be based on the calculated, not requested,
 *   pixel clock. Added f_max = maximum f_vco frequency.
 *
 *   October 19, 1996 - [aem] Andrew E. Mileski
 *   Commented the loop clock code (wow, I understand everything now),
 *   and simplified it a bit. This should really be two separate functions.
 *
 *   October 1, 1996 - [aem] Andrew E. Mileski
 *   Optimized the m & n picking algorithm. Added maxClock detection.
 *   Low speed pixel clock fix (per the docs). Documented what I understand.
 *
 *   ?????, ??, ???? - [???] ????????????
 *   Based on the TVP3026 code in the S3 driver.
 */

void MGATi3026SetPCLK( f_out, bpp )
	long	f_out;
	int	bpp;
{
        /* DAC values */
        unsigned char DACclk[3];
        int i;

	/* Pixel clock values */
	int m, n, p;

	/* The actual frequency output by the clock */
	double f_pll;

	/* Get the maximum pixel clock frequency */
	long f_max = TI_MAX_VCO_FREQ;
	if ( 170000 /*vga256InfoRec.maxClock*/ > TI_MAX_VCO_FREQ )
		f_max = 170000/*vga256InfoRec.maxClock*/;

	/* Do the calculations for m, n, and p */
	f_pll = MGATi3026CalcClock( f_out, f_max, & m, & n, & p );

	/* Values for the pixel clock PLL registers */
	DACclk[ 0 ] = ( n & 0x3f ) | 0xc0;
	DACclk[ 1 ] = ( m & 0x3f );
	DACclk[ 2 ] = ( p & 0x03 ) | 0xb0;

#ifdef DEBUG
	ErrorF( "bpp=%d z=%.1f ln=%d lm=%d lp=%d lq=%d\n",
		bpp, z, ln, lm, lp, lq );
#endif
        MGAoutTi3026(0x2C, 0xfe);
        MGAoutTi3026(0x2D, 0);	/* p value to 0 */
        MGAoutTi3026(0x2C, 0xfc);
        for (i = 0; i < 3; i++)
                MGAoutTi3026(0x2D, DACclk[i]);
/*	MGATi3026SetMCLK( 60000L ); */
        MGAoutTi3026(0x0f, 0x06);
        MGAoutTi3026(0x18, 0x80);
        MGAoutTi3026(0x19, 0x98);
        MGAoutTi3026(0x1A, 0x00);
        MGAoutTi3026(0x1C, 0x00);
        MGAoutTi3026(0x1D, 0x00);
        MGAoutTi3026(0x1E, 0x00);
        MGAoutTi3026(0x38, 0x00);
        MGAoutTi3026(0x39, 0x18);
	/* reset CRTC extention registers */
	for (i = 0; i < 6; i++) {
	   outb(0x3de, i);
	   outb(0x3df, 0);
	}
	/* sequencer registers */
	outb(0x3c4, 1);
	outb(0x3c5, inb(0x3c5) & 0xe3);
}
