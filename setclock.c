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
 *** main SVGA clock programming functions for SVGATextMode
 ***/

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "misc.h"
#include "vga_prg.h"
#include "wait_vsync.h"
#include "setclock.h"
#include "std_clock.h"
#include "clockchip.h"
#include "messages.h"  
#include "run_extprog.h"  

/*****************************************************************************************************************************/

void clock_check(int result)
{
  if (result < 0)
  {
    switch(result)
    {
      case CLKSEL_DONOTHING:     PWARNING(("Clock selection: Warning: Clock not changed\n"));
                                 break;
      case CLKSEL_ILLEGAL_NUM:   PERROR(("Clock selection: illegal clock number.\n"));
                                 break;
      case CLKSEL_NULLCLOCK:     PERROR(("Clock selection: 0 MHz clock selected! It would lock up the video card. Aborting...\n"));
                                 break;
      case CLKSEL_OUT_OF_BOUNDS: PERROR(("Clock selection: Requested clock frequency out of bounds\n"));
                                 break;
      default: PERROR(("Clock selection: unknown error\n"));
    }
  }
}


int findclosestclock(int req_clock, int *closest_clock)
{
   /* returns closest clock NUMBER when one is found, error code otherwise */

   int i, closest=0;
   int num=clock_data.num_clocks;

   *closest_clock = req_clock;  /* start with both equal */

   if (req_clock == 0) return(CLKSEL_NULLCLOCK);
   
   /* create 1/2 clocks when option is set */
   if (OFLG_ISSET(OPT_CLOCKDIV2))
   {
     for (i=0; i<num; i++) clock_data.clocks[i+num] = clock_data.clocks[i]/2;
     num *= 2;
   }

   /* for (i=0; i<num; i++) PDEBUG(("CLOCK #%d = %d\n",i, clock_data.clocks[i])); */
   
   /* find closest clock frequency */
   for (i=0 ; i < num ; i ++)
   {
      if ( abs(clock_data.clocks[i] - req_clock) < abs(clock_data.clocks[closest] - req_clock) ) { closest = i; }
   }

   *closest_clock = clock_data.clocks[closest];
   PDEBUG(("findclosestclock: closest clock nr %d = %d kHz.\n",closest, *closest_clock));
   if (closest>clock_data.num_clocks)
     PDEBUG((" (clock index #%d is actually index #%d divided by 2)\n", closest, closest-clock_data.num_clocks));
   
   if (closest < 0) return(CLKSEL_DONOTHING);
   if (closest > num) return(CLKSEL_ILLEGAL_NUM);
   if (*closest_clock == 0) return(CLKSEL_NULLCLOCK);
   
   return(closest);
}


int GetClock(int chipset, int freq, int *closestfreq, int report_error)
 /* look for correct clock index (for indexed clock chips), and check for out-of-bound clocks */
{
   int result;
   
   switch(chipset)
   {
     case CS_CIRRUS: /* should do some range checking here */
                     result = 0; /* suppose it can make any frequency */
                     break;
     default: result = findclosestclock(freq, closestfreq);
   }
   if (report_error) clock_check(result);
   return(result);
}



/* This is only for non-programmable clockchips! */
void SetClock(int chipset, int freq)
{
   int result, divby2=0;
   
   result = GetClock(chipset, freq, &freq, TRUE);
  /* clock number is supposed to be in the allowable range by now.
     error checking should have been done before.
     No error checking will be done in clock-setting routine! */
     
   if (OFLG_ISSET(OPT_CLOCKDIV2) && (result > clock_data.num_clocks-1))
   {
      divby2 = 1;
      result -= clock_data.num_clocks; /* make clock selection routine pick the real clock (before division by 2) */
   }

   PDEBUG(("Setting Clock to %d kHz\n", freq));
   SYNCRESET_SEQ;
   switch(chipset)
   {
     case CS_VGA:
     case CS_NEOMAGIC:
        SET_CLOCKBITS_0_1(result);
        break;
     case CS_CIRRUS:
        CirrusClockSelect(freq, (clock_data.clockchiptype == CLKCHIP_LAGUNA));
        break;
     case CS_S3:
        s3ClockSelect(result);
        break;
     case CS_ET4000:
        ET4000ClockSelect(result);
        break;
     case CS_ET6000:
        ET6000ClockSelect(result);
        break;
     case CS_ET3000:
        ET3000ClockSelect(result);
        break;
     case CS_TVGA8900:
     case CS_TVGA9000:
        TVGAClockSelect(chipset, clock_data.num_clocks, result);
        break;
     case CS_PVGA1:
     case CS_WDC90C0X:
     case CS_WDC90C1X:
     case CS_WDC90C2X:
     case CS_WDC90C3X:
        WDCClockSelect(chipset, clock_data.num_clocks, result);
        break;
     case CS_ATI:
     case CS_ATIMACH32:
     case CS_ATIMACH64:
        ATIClockSelect(chipset, result);
        break;
     case CS_VIDEO7:
        Video7ClockSelect(result);
        break;
     case CS_ALI:
     case CS_AL2101:
        ALIClockSelect(chipset, result);
        break;
     case CS_OTI67:
     case CS_OTI77:
     case CS_OTI87:
        OAKClockSelect(chipset, result);
        break;
     case CS_SIS:
        SISClockSelect(result);
        break;
     case CS_REALTEK:
        RealTekClockSelect(result);
        break;
     case CS_ARK:
        ARKClockSelect(result);
        break;
     case CS_NCR22E:
     case CS_NCR32:
        NCRClockSelect(chipset, result);
        break;
     case CS_GVGA:
        GVGAClockSelect(result);
        break;
     case CS_MX:
        MXClockSelect(result);
        break;
     case CS_TGUI:
	TGUIClockSelect(result);
	break;
     case CS_MATROX:
	MatroxClockSelect(result);
	break;
     case CS_RIVA128:
	RIVA128ClockSelect(freq);
	break;
     default: PERROR(("setclock.c: internal error: unknown chip set #%d\n", chipset));
   }

   if (OFLG_ISSET(OPT_CLOCKDIV2))
   {
      Outbit_SEQ(1,3,divby2);
      if (divby2) PDEBUG(("Clock (%d) needed 'division by 2' feature.\n", freq));
   }
   else
   {
      /* disable ClockDiv2 always */
      Outbit_SEQ(1,3,0);
   }

   ENDRESET_SEQ;
   usleep(50000); /* let PLL clock synthesizer stabilize */
   clock_check(result);
}


/*****************************************************************************************************************************/

void do_clock(int chipset, int clock, int charwidth)
{
  char tempstr[1024]="";

  /* program clocks in V-blanking only, avoiding video memory corruption and system hangs (?) */

  safe_wait_vsync(); /* wait for VSYNC... if there is one */
  SCREEN_OFF;

  /* mclk programming */
  if (clock_data.mclk != MCLK_NOT_DEFINED)
  {
    if (clock_data.clockchiptype != CLKCHIP_NONE)
    {
      /* error flagging for clockchips that don't support MCLK programming is done in validate.c */
      set_clockchip_Mclock(chipset, clock_data.mclk);
      PMESSAGE(("Setting mclk to %d kHz\n", clock_data.mclk));
    }
    else PERROR(("mclk programming not supported for non-clockchip VGA cards\n"));
  }

  /* we'll always set the MATROX MClk to a useful default (the BIOS sets it too low) */
  if ( (chipset == CS_MATROX) && (clock_data.mclk == MCLK_NOT_DEFINED) )
  {
    clock_data.mclk = 60000;
  }

  safe_wait_vsync(); /* wait for VSYNC... if there is one */
  
  /* PixClock programming */
  if (clock_data.ck_prog_path)
  {
     int clockindex = 0;
     /* find clock index in clocks line */
     clockindex = GetClock(chipset, clock, &clock, TRUE);
     snprintf(tempstr, sizeof(tempstr), "%s %1.3f %d", clock_data.ck_prog_path, clock/1000.0, clockindex);
     SYNCRESET_SEQ;
     Run_extern_Prog(tempstr);
     ENDRESET_SEQ;
  }
  else
    if (clock_data.clockchiptype != CLKCHIP_NONE)
      set_clockchip_clock(chipset, clock, charwidth);
  else
  {
    SetClock(chipset, clock);
  }
  SCREEN_ON;
}

 
