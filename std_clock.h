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
 ***/

#ifndef _STD_CLOCK_H
#define _STD_CLOCK_H

void SET_CLOCKBITS_0_1(int no);

void LegendClockSelect(int no);

void TGUIClockSelect(int no);

void TVGAClockSelect(int chipset, int num_clocks, int no);

void s3ClockSelect(int no);

void ATIClockSelect(int chipset, int no);

void WDCClockSelect(int chipset, int num_clocks, int no);

void ET4000ClockSelect(int no);

void ET6000ClockSelect(int no);

void ET3000ClockSelect(int no);

void CirrusClockSelect(int freq, bool islaguna);

void Video7ClockSelect(int no);

void ALIClockSelect(int chipset, int no);

void OAKClockSelect(int chipset, int no);

void SISClockSelect(int no);

void RealTekClockSelect(int no);

void ARKClockSelect(int no);

void NCRClockSelect(int chipset, int no);

void GVGAClockSelect(int no);

void MXClockSelect(int no);

void MatroxClockSelect(int no);

#endif

