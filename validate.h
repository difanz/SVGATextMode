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
 *** validate.h, mode validation functions
 ***/

#ifndef _VALIDATE_H
#define _VALIDATE_H

#include "cfg_structs.h"
#include "chipset.h"

/* These limits are partly based on guessing and partly on experience ... */
#define MAX_CLOCK_DEVIATION  1500    /* kHz */
/* only used when one single frequency is specified in config file */
#define MAX_HSYNC_DEVIATION   600    /* Hz */
#define MAX_VSYNC_DEVIATION  1200    /* 1/1000 Hz */

void sanitize_cfgfile_data(void);

bool check_range(int checkval, t_mon_timing *p_tim);

bool check_clockgen(int req_clock, int report_error);

bool validate_clock(int req_clock, int report_error);

void scan_valid_modes(int validate);

void check_and_show_mode(t_mode* p_mode, int checkit);

#endif
