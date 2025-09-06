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
 *** dump_cfg_data.c: debugging function. Dumps config file data.
 ***/


#include <stdio.h>
#include "misc.h"
#include "chipset.h"
#include "messages.h"

#define NOTDEFINED "(Not Defined)"

void dump_mode(t_mode *m)
{
   PDEBUG(("  `%s' [%dx%d]  %dkHz  %d %d %d %d  %d %d %d %d  font %dx%d  flags=0x%X  H/V=%.1f/%.1f\n",
      m->name,
      m->cols, m->rows, 
      m->pixelClock,
      m->HDisplay, m->HSyncStart, m->HSyncEnd, m->HTotal,
      m->VDisplay, m->VSyncStart, m->VSyncEnd, m->VTotal,
      m->FontWidth, m->FontHeight,
      m->flags,
      m->hfreq/1000.0, m->vfreq/1000.0
    ));
}

void dump_cfgdata(int dbg_lev)
{
  int i;
  t_mode *curr_textmode;
  t_mon_timing *curr_tim;
  t_terminals *curr_term;
  
  if (dbg_lev>0)
  {
    PDEBUG(("parsed data from config file:\n"));
    PDEBUG(("Chipset = %d (%s)\n", chipset, ChipsetRec[chipset].name_str));
    
    if (dbg_lev>1)
    {
      PDEBUG(("Modelines:\n"));
      curr_textmode = text_mode_list;
      while (curr_textmode) 
      {
        dump_mode(curr_textmode);
        curr_textmode = curr_textmode->next;
      }
    }

    PDEBUG(("monitor H limits:"));
    curr_tim = h_mon_limits;
    while (curr_tim)
    {
      fprintf(stderr, " %d - %d %c", curr_tim->low_limit, curr_tim->high_limit, curr_tim->next ? ',' : '\n');
      curr_tim = curr_tim->next;
    }
    
    PDEBUG(("monitor V limits:"));
    curr_tim = v_mon_limits;
    while (curr_tim)
    {
      fprintf(stderr, " %d - %d %c", curr_tim->low_limit, curr_tim->high_limit, curr_tim->next ? ',' : '\n');
      curr_tim = curr_tim->next;
    }

    PDEBUG(("Clocks:"));
    for (i=0; i<clock_data.num_clocks; i++) fprintf(stderr," %d", clock_data.clocks[i]);
    fputc('\n', stderr);
    
    PDEBUG(("DacSpeed = %dkHz\n", clock_data.maxclock));

    if (clock_data.mclk!=MCLK_NOT_DEFINED) PDEBUG(("MClk     = %dkHz\n", clock_data.mclk));
      else PDEBUG(("MClk not defined\n"));

    if (clock_data.refclk!=REFCLK_NOT_DEFINED) PDEBUG(("RefClk   = %dkHz\n", clock_data.refclk));
      else PDEBUG(("RefClk not defined\n"));

    if (clock_data.clockchiptype!=CLKCHIP_NONE)
      PDEBUG(("ClockChip = %d (%s)\n", clock_data.clockchiptype, ClockchipRec[clock_data.clockchiptype].name_str));
      else PDEBUG(("ClockChip not defined\n"));

    PDEBUG(("Optmask = 0x%x\n", STM_Options));

    PDEBUG(("ResetProg = `%s'\n", resetprogpath ? resetprogpath : NOTDEFINED));

    PDEBUG(("DefaultMode = `%s'\n", defaultmode ? defaultmode : NOTDEFINED));

    PDEBUG(("Underline_pos = %d%s\n", underline_pos, (underline_pos<0) ? " (disabled)" : ""));

    PDEBUG(("ClockProg = `%s'\n", clock_data.ck_prog_path? clock_data.ck_prog_path : NOTDEFINED));

    PDEBUG(("Terminals: "));
    if (!(curr_term = p_terminals)) fprintf(stderr,NOTDEFINED);
    while (curr_term)
    {
      fprintf(stderr, "%s ", curr_term->name);
      curr_term = curr_term->next;
    }
    fputc('\n', stderr);
    
    PDEBUG(("cursor_start = %d ; cursor_end = %d\n", cursor_start, cursor_end));
    
    PDEBUG(("bordercolor  = %d\n", bordercolor));

    PDEBUG(("FontProg = `%s'\n", font_data.fontprogpath ? font_data.fontprogpath : NOTDEFINED));
    PDEBUG(("FontPath = `%s'\n", font_data.fontpath ? font_data.fontpath : NOTDEFINED));
    
    if (dbg_lev>1)
    {
      PDEBUG(("Font selections:\n"));
      for (i=0; i<32; i++)
      {
        if (font_data.font_table[0][i]) PDEBUG(("8x%d : %s\n", i+1, font_data.font_table[0][i]));
        if (font_data.font_table[1][i]) PDEBUG(("9x%d : %s\n", i+1, font_data.font_table[1][i]));
      }
    }
  
  }
}
 