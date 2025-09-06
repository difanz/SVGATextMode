/* svga.h: register definitions for SVGAs
 * This file is part of vgaset.
 * 
 * vgaset is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * vgaset is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with vgaset; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * Greg Lehey, 29 January 1993
 * XXX Still almost completely incomplete
 */
#define ATTRIBUTE_INDEX 0x3c0
#define ATTRIBUTE_DATA	0x3c1
#define MISC_OUTPUT	0x3c2
#define INPUT_STATUS_1	0x3c2
#define INPUT_STATUS_2	0x3da
#define FEATURE_CONTROL_OUT	0x3da
#define FEATURE_CONTROL_IN	0x3ca
/* sequencer: write index to INDEX, register data to DATA */
#define SEQUENCER_INDEX	0x3c4
#define SEQUENCER_DATA	0x3c5
#define		RESET_REGISTER		0
#define		CLOCK_MODE_REGISTER	1
#define		MAP_MASK_REGISTER	2
#define		MAP_SELECT_REGISTER	3
#define		MEMORY_MODE_REGISTER	4
/* CRT controller registers */
#define CRTCL_INDEX	(vgaIOBase + 4)
#define CRTCL_DATA	(vgaIOBase + 5)
/* the following registers contain a character count or a pixel count >> 3, depending on the operating
 * mode. They reference 0 as the start of the displayed data */
#define		HT	0				    /* horizontal total */
#define		HDE	1				    /* horizontal display end */
#define		SHB	2				    /* start horizontal blanking */
#define		EHB	3				    /* end horizontal blanking */
#define		SHR	4				    /* start horizontal retrace */
#define		EHR	5				    /* end horizontal retrace */
/* These define the vertical mode. They are line counts starting at the first displayed line */
#define		VT	6				    /* total lines per frame */
#define		OVERFLOW 7				    /* overflow for some other registers */
#define		PRS	8				    /* preset row scan register */
#define		MSL	9				    /* maximum scan lines per character */
#define		CS	10				    /* cursor start line within character scan */
#define		CE	11				    /* cursor end line */
#define		SAH	12				    /* high-order start address of display */
#define		SAL	13				    /* start address low */
#define		CLH	14				    /* cursor location high */
#define		CLL	15				    /* cursor location low */
#define		VRS	16				    /* vertical retrace start - lines */
#define		EVR	17				    /* end vertical retrace */
#define		VDE	18				    /* vertical display end */
#define		OFF	19				    /* offset register: difference in
							     * address between two vertically neighbouring
							     * pixels */
#define 	UL	20				    /* underling location register */
#define		VBS	21				    /* vertical blank start */
#define		VBE	22				    /* vertical blank end */
#define		MODE_CONTROL	23			    /* mode control: */
#define			MC_HR	0x80			    /* hardware reset */
#define			W_B	0x40			    /* word/byte mode */
#define			AW	0x20			    /* address wrap mode */
/* 0x10 not used in VGA */
#define			CBT	0x8			    /* count by two increment mode */
#define			HRS	0x4			    /* horizontal retrace select */
#define			SRS	0x2			    /* select row scan counter */
#define			CMS	0x1			    /* compatibility mode support */
#define		LC	24				    /* line compare register */
#define	CRTC_REG_COUNT	25				    /* number of sequencer registers */

#define TSENG_OVERFLOW_HIGH	0x35			    /* Tseng T3000/T4000: overflow high reg */
#define 	TSENG_INTERLACE	0x80			    /* bit to set interlace */
