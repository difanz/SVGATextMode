/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/t89_driver.c,v 3.48 1996/10/24 14:25:20 dawes Exp $ */

/* $XConsortium: t89_driver.c /main/19 1996/01/26 14:29:55 kaleb $ */

/*
 * Programming the built-in Trident TGUI clock generator.
 * 
 * Ported from XFree 3.2 to SVGATextMode, with some code fixes,
 * by Massimiliano Ghilardi <max@Linuz.sns.it>
 *
 * This driver is for the ChipSet "TGUI".
 * It supports the following programmable clockchips:
 * TGUI9320LCD                        :  use ClockChip "TGUI9320"
 * TGUI9440AGi, TGUI9660XGi, TGUI9680 :  use ClockChip "TGUI9440"
 * CYBER 938x                         :  use ClockChip "CYBER938x"
 * 
 * It also supports the following non-programmable chipsets:
 * TGUI9400CXi, TGUI9430DGi, TGUI9420, TGUI9420DGi
 * _don't_ use a ClockChip at all for these, but a Clocks line!
 * 
 * For Trident TVGA chipsets (not TGUI) use TVGA8900 / TVGA9000.
 */

#include "chipset.h"
#include "compiler.h"
#define NO_OSLIB_PROTOTYPES
#include "xf86_OSlib.h"

/*
 * TGUISetClock -- Set programmable clock for TGUI cards !
 */
void TGUISetClock(int freq) {
	
	int clock_diff = 500;
	int ffreq;
	int m, n, k;
	int p, q, r, s; 
	int startn, endn;
	int endm;
	int startk = 1, endk = 3;
	unsigned char temp;

	p = q = r = s = 0;

	if (clock_data.clockchiptype==CLKCHIP_CYBER938x)
	{
		startn = 64;
		endn = 255;
		endm = 64;
	}
	else
	{
		startn = 1;
		endn = 122;
		endm = 32;
	}

	if (freq < 16000)
		startk = 2;
	else if (freq > 75000)
		endk = 2;
	
	for (k=startk;k<endk;k++)
	  for (n=startn;n<endn;n++)
	    for (m=1;m<endm;m++)
	    {
		ffreq = ((( (n + 8) * 14.31818) / ((m + 2) * k)) * 1000);
		if ((ffreq > freq - clock_diff) && (ffreq < freq + clock_diff)) 
		{
			p = n; q = m; r = k; s = ffreq;
			clock_diff = (freq > ffreq) ? freq - ffreq : ffreq - freq;
		}
	    }

	if (clock_data.clockchiptype!=CLKCHIP_TGUI9320) { /* NOT TGUI 9320 LCD */
		temp = inb(0x3CC);
		outb(0x3C2,  /* VCLK_O */ (temp & 0xF3) | 0x08);
	}
	
	if (clock_data.clockchiptype==CLKCHIP_CYBER938x)
	{
		outb(0x43C8, /* VCLK_A */ p);
		outb(0x43C9, /* VCLK_B */ (q & 0x2F) | ((r - 1) << 6));
	}
	else
	{
		outb(0x43C8, /* VCLK_A */ ((1 & q) << 7) | p);
		outb(0x43C9, /* VCLK_B */ (((q & 0xFE) >> 1) | ((r - 1) << 4)));
	}

}
