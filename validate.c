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
 *** validate.c, mode validation functions
 ***/

/*
 * If defined, CHECK_NUMCLOCKS will check if the number of clocks in the Clocks line
 * corresponds to the number that SVGATextMode expects. This is more of a hassle than an advantage,,,
 */
#define CHECK_NUMCLOCKS
   

#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "validate.h"
#include "cfg_structs.h"
#include "setclock.h"
#include "messages.h"
#include "chipset.h"


void sanitize_cfgfile_data(void)
{
  if (chipset<0) PERROR(("No chipset defined in config file\n"));
  if (!h_mon_limits) addhsync(DEFLT_HSYNC_MIN, DEFLT_HSYNC_MAX);
  if (!v_mon_limits) addvsync(DEFLT_VSYNC_MIN, DEFLT_VSYNC_MAX);
  /* some chips always come with a built-in clockchip, so we can force that here */
  if (clock_data.clockchiptype==CLKCHIP_NONE)
    switch (chipset)
    {
      case CS_CIRRUS:
        clock_data.clockchiptype = CLKCHIP_CIRRUS;
        break;
      case CS_ET6000:
        clock_data.clockchiptype = CLKCHIP_ET6000;
        break;
      case CS_RIVA128:
        clock_data.clockchiptype = CLKCHIP_RIVA128;
        break;
    }
  if ((clock_data.num_clocks==0) && (clock_data.clockchiptype==CLKCHIP_NONE) && (!clock_data.ck_prog_path))
    PERROR(("No Clocks line, no ClockChip and no ClockProg defined in config file. Make up your mind.\n"));
  if ((clock_data.num_clocks!=0) && (clock_data.clockchiptype!=CLKCHIP_NONE))
    PERROR(("Both ClockChip _and_ a Clocks line defined in config file. Make up your mind.\n"));
  /* doing this check here instead of in clockchip.c avoids half-programmed textmode aborts */
  if (clock_data.mclk!=MCLK_NOT_DEFINED)
    switch(clock_data.clockchiptype)
    {
      case CLKCHIP_S3_SDAC:
      case CLKCHIP_S3GENDAC:
      case CLKCHIP_ICS5300:
      case CLKCHIP_ICS5342:
      case CLKCHIP_ICS5341:
      case CLKCHIP_S3TRIO:
      case CLKCHIP_ET6000:
      case CLKCHIP_TI3026:
         break;  /* these chips support clockchip programming */
      default:
         PERROR(("MClk programming not supported on selected clockchip\n"));
    }
  if ((clock_data.clockchiptype==CLKCHIP_IBMRGB5XX) && (clock_data.refclk==REFCLK_NOT_DEFINED))
    PERROR(("S3 cards with IBM RGB RAMDAC must have RefClk defined in config file.\n"));
  if ( ( !(clock_data.clockchiptype==CLKCHIP_TI3026) && !(clock_data.clockchiptype==CLKCHIP_TI3025) )
       && (OFLG_ISSET(OPT_SOG)) )
    PERROR(("The S3 `sync_on_green' option is only allowed for TI302X RAMDAC's\n"));

#ifdef CHECK_NUMCLOCKS  
   /*
    * check for correct amount of clocks, if possible for the specified chip set. This could be incorrect...
    */

#define CHECKCLK(nclk) \
    if ( clock_data.num_clocks != (nclk) ) \
      PWARNING(("`%s' chipset normally has %d clocks (currently %d defined).\n",\
                 ChipsetRec[(chipset)].name_str, (nclk), clock_data.num_clocks))
                 
    switch (chipset)      
    {
      /* first, the exceptions */
      case CS_VGA:
        if (clock_data.num_clocks > 4)
          PWARNING(("Generic VGA chipsets can have no more than 4 clocks (currently %d defined).\n", clock_data.num_clocks));
        break;
      case CS_S3: 
        if ((clock_data.num_clocks>0) && (clock_data.num_clocks != 16))
          PWARNING(("`S3' chipsets (without clockchip) normally have 16 clocks (currently %d defined).\n", clock_data.num_clocks));
        break;
      case CS_ET6000: 
        if ((clock_data.num_clocks>0) && (clock_data.num_clocks != 8))
          PWARNING(("`ET6000' chipsets (without clockchip) normally have 8 clocks (currently %d defined).\n", clock_data.num_clocks));
        break;
      case CS_PVGA1:
        if ((clock_data.num_clocks != 8) && (clock_data.num_clocks != 4))
          PWARNING(("`PVGA1' chipset must have 4 or 8 clocks in 'clocks' line (currently %d defined).\n", clock_data.num_clocks));
        break;

      /* generic cases */

      case CS_WDC90C0X:
      case CS_WDC90C1X:
      case CS_WDC90C2X:
        CHECKCLK( 9);
        break;

      case CS_WDC90C3X:
        CHECKCLK(17);
        break;

      case CS_ATIMACH32:
        CHECKCLK(32);
        break;

      case CS_ATI:
        CHECKCLK(64);
        break;

      case CS_NEOMAGIC:
        CHECKCLK(4);
        break;

      case CS_ALI:
      case CS_OTI87:
      case CS_SIS:
      case CS_REALTEK:
      case CS_ARK:
      case CS_NCR32:
        CHECKCLK(16);
        break;

      case CS_AL2101:
      case CS_OTI67:
      case CS_OTI77:
      case CS_GVGA:
      case CS_NCR22E:
      case CS_MX:
      case CS_ET3000:
        CHECKCLK( 8);
        break;
    }
#endif         

}

bool check_range(int checkval, t_mon_timing *p_tim)
{
  bool range_ok=FALSE;
  t_mon_timing *curr_tim = p_tim;
  
  while ((curr_tim) && (!range_ok))
  {
    range_ok = ((checkval >= curr_tim->low_limit) && (checkval <= curr_tim->high_limit));
    PDEBUG(("Check_range(%d): range from %d to %d %s\n", checkval, curr_tim->low_limit, curr_tim->high_limit, (range_ok==TRUE) ? "OK" : "failed"));
    curr_tim = curr_tim->next;
  }
  return(range_ok);
}


bool validate_clock(int req_clock, int report_error)
{
  int temp;  
  int realclock;

  /* find closest clock */
  if (GetClock(chipset, req_clock, &realclock, report_error) < 0)
    return(FALSE);

  /* calculate deviation from requested clock */
  temp = abs(realclock-req_clock);

  if( temp > MAX_CLOCK_DEVIATION )
  {
    if (report_error)
    {
      if ((req_clock < 25000) && (!OFLG_ISSET(OPT_CLOCKDIV2)))
        PWARNING(("Selected clock is below standard VGA clocks, and is not available in 'clocks' line.\n\
                   UNLESS you enable division by 2 (Option 'ClockDiv2')\n"));
      PERROR(("The closest available clock %.2f differs too much (max = %.1f) from specified clock %.2f\n",\
               realclock/1000.0, MAX_CLOCK_DEVIATION/1000.0, req_clock/1000.0));
    }
    return(FALSE);
  }
  PDEBUG(("Clock deviation (from requested clock) = |%d-%d|=%d kHz (%1.2f%%).\n",
           req_clock, realclock, temp, (temp*100.0)/req_clock));

  return(TRUE);
}


/*
 * check clock generator limits
 */

int check_clockgen(int req_clock, bool report_error)
{
  PDEBUG(("Checking if %d kHz can be achieved\n", req_clock));
  if (clock_data.clockchiptype == CLKCHIP_NONE)
  {
    if (!validate_clock(req_clock, report_error)) return(FALSE);
  }
  else
  {
    if (req_clock < ClockchipData[clock_data.clockchiptype].minclock / 2) {
      PDEBUG(("Check Clockchip speed: too low\n"));
      if (report_error)
        PERROR(("Selected clockchip (%s) cannot produce requested clock of %.2f MHz (min = %.2f)\n",\
                 ClockchipRec[clock_data.clockchiptype].name_str,\
                 req_clock/1000.0,\
                 ClockchipData[clock_data.clockchiptype].minclock/2000.0));
      return(FALSE);
    }
    if (req_clock < ClockchipData[clock_data.clockchiptype].minclock && (!OFLG_ISSET(OPT_CLOCKDIV2)))
    {
      PDEBUG(("Check Clockchip speed: too low\n"));
      if (report_error)
        PERROR(("Selected clockchip (%s) cannot produce requested clock of %.2f MHz (min = %.2f)\n\
                 UNLESS you enable division by 2 (Option 'ClockDiv2')\n",\
                 ClockchipRec[clock_data.clockchiptype].name_str,\
                 req_clock/1000.0,\
                 ClockchipData[clock_data.clockchiptype].minclock/1000.0));
      return(FALSE);
    }
    if (req_clock > ClockchipData[clock_data.clockchiptype].maxclock)
    {
      PDEBUG(("Check Clockchip speed: too high\n"));
      if (report_error)
        PERROR(("Selected clockchip (%s) cannot produce requested clock of %.2f MHz (max = %.2f)\n",\
                 ClockchipRec[clock_data.clockchiptype].name_str,\
                 req_clock/1000.0,\
                 ClockchipData[clock_data.clockchiptype].maxclock/1000.0));
      return(FALSE);
    }
  }
  return(TRUE);
}


/*
 * validate the mode line, using H/V freq limits, clock definitions and max clock
 * abort with error message or just return error code depending on "report_error" flag
 * Also check if the required clock (or a close match) is available from clock generator.
 */


int validate_mode(t_mode *mode, bool report_error)
{
  int max_clock;

 /*
  * Check VGA chip clock speed limit.
  */

  /* adjust clock speed for clocks-per-character, aka "character bandwidth" */
  max_clock = (clock_data.maxclock * mode->FontWidth)/8;
   
  if ( mode->pixelClock > max_clock )
  {
    PDEBUG(("Check clock speed: Too high\n"));
    if (report_error)
      PERROR(("Pixel Clock (%.2f MHz) too high for this chipset\n"\
              "  max pixel clock = %.2f/%.2f MHz for 8/9 pixel wide font resp.\n",\
               mode->pixelClock/1000.0, clock_data.maxclock/1000.0, clock_data.maxclock*9/8000.0));
    return(FALSE);
  }
  else PDEBUG(("Check max clock speed: OK\n"));

 /*
  * Check clock generator limits.
  */

  if (!check_clockgen(mode->pixelClock, report_error)) return(FALSE);

 /*
  * Check monitor H/V freq limits.
  */

  if (!check_range(mode->hfreq, h_mon_limits)) 
  {
    if (report_error) PERROR(("Horizontal Sync Frequency (%.2fkHz) out of range.\n", mode->hfreq/1000.0));
    return(FALSE);
  }
  if (!check_range(mode->vfreq, v_mon_limits))
  {
    if (report_error) PERROR(("Vertical Refresh Frequency (%.2fHz) out of range.\n", mode->vfreq/1000.0));
    return(FALSE);
  }
  return(TRUE);
}


void scan_valid_modes(int validate)
{
  t_mode *curr_textmode;

  PDEBUG(("Scanning for valid Text Mode lines\n"));

  curr_textmode = text_mode_list;
  while (curr_textmode)
  {
    if ((!validate) || validate_mode(curr_textmode, FALSE))
    {
      printf("%s  Clock: %.2fMHz  Size: %dx%d  CharCell: %dx%d%s  Refresh: %.2fkHz/%.1fHz\n",
              curr_textmode->name, curr_textmode->pixelClock/1000.0,
              curr_textmode->cols, curr_textmode->rows,
              curr_textmode->FontWidth, curr_textmode->FontHeight,
              MOFLG_ISSET(curr_textmode,ATTR_DOUBLESCAN) ? "D" : "", curr_textmode->hfreq/1000.0, curr_textmode->vfreq/1000.0);
    }
    curr_textmode = curr_textmode->next;
  }
}

void check_and_show_mode(t_mode* p_mode, int checkit)
{
 /*
  * First show what mode would be programmed.
  */
  printf("Chipset = `%s', Textmode clock = %.2f MHz, %dx%d chars, CharCell = %dx%d%s. Refresh = %3.2fkHz/%3.1fHz.\n",
          ChipsetRec[chipset].name_str,p_mode->pixelClock/1000.0,p_mode->cols,p_mode->rows,
          p_mode->FontWidth, p_mode->FontHeight,MOFLG_ISSET(p_mode,ATTR_DOUBLESCAN) ? "D" : "",
          p_mode->hfreq/1000.0,p_mode->vfreq/1000.0);
 /*
  * Now we should do some checking to see if the horizontal and vertical refresh frequencies are within limits
  * Don't do this when we will not be programming the hardware, so you can check a mode's frequencies with "-n".
  */
  if (checkit) validate_mode(p_mode, TRUE);
}
