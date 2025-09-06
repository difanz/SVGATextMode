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
 *** Config file parsing structures and data types for SVGATextMode.
 *** Used by lex/yacc parser.
 ***
 ***/
 
#include "misc.h"

#ifndef _CFG_STRUCTS_H
#define _CFG_STRUCTS_H

/*** mode line attributes ***/

#define ATTR_INTERLACE   1<<0
#define ATTR_DOUBLESCAN  1<<1

/*** monitor refresh limits struct ***/

#define MAX_RANGE_SPECS  32

#define DEFLT_HSYNC_MIN  30.0
#define DEFLT_HSYNC_MAX  32.0
#define DEFLT_VSYNC_MIN  45.0
#define DEFLT_VSYNC_MAX  80.0

typedef struct _TMT t_mon_timing;
struct _TMT {
    int low_limit;
    int high_limit;
    t_mon_timing *next;
};

typedef struct _TERMDEF t_terminals;
struct _TERMDEF {
    char *name;
    t_terminals *next;
};

typedef struct t_fontdef {
        char *fontpath;
        char *fontprogpath;
        char *font_table[2][32];          /* font string table */
} t_fontdef;

#define DEFAULT_FONTPROGPATH  "/usr/bin/setfont"
#define DEFAULT_FONTPATH      NULL

#define INIT_FONTDATA { DEFAULT_FONTPATH, DEFAULT_FONTPROGPATH, }

/*** mode line definitions ***/

typedef enum e_sync_pol  { POS=1 , NEG } sync_pol ;

typedef struct _TTM t_mode;
struct _TTM {
    char *name;
    int pixelClock;                                 /* Pixel clock in kHz. */
    int HDisplay, HSyncStart, HSyncEnd, HTotal;     /* Horizontal Timing. */
    int HBlankStart, HBlankEnd;
    int VDisplay, VSyncStart, VSyncEnd, VTotal;     /* Vertical Timing. */
    int VBlankStart, VBlankEnd;
    int FontWidth, FontHeight;                      /* font size */
    sync_pol hpol, vpol;                            /* sync polarities */
    int hshift;					    /* Leftward screen shift, in chars */
    int hfreq, vfreq;                               /* refresh frequencies in Hz(H) and milliHerz(V) */
    int cols, rows;                                 /* screen geometry */
    int flags;
    t_mode *next;
};

#define MOFLG_SET(m,opt)    ((m)->flags |= (opt))
#define MOFLG_ISSET(m,opt)  ( ((m)->flags & (opt)) != 0 )

/*** clock config struct ***/

#define MAX_CLOCKS              128   /* I suppose there are no chipsets with more clocks out there */

/* Some very arbitrary clock limits */
#define MIN_CLOCK               0.0   /* must be 0 to allow "dummy" clock values in clocks line */
#define MAX_CLOCK               500.0

#define MIN_MCLK		30.0
#define MAX_MCLK		80.0

#define MIN_RCLK		4.0
#define MAX_RCLK		62.0

#define MCLK_NOT_DEFINED  -1.0 
#define REFCLK_NOT_DEFINED  -1.0 
#define DEFAULT_MAXCLOCK  45000

/* the return structure. Only part of it will be used, depending on clock type */
typedef struct t_clockdef {
    int num_clocks;              /* number of clocks in clocks line, and thus in array below */
    int clockchiptype;           /* if used: ClockChip type */
    int maxclock;                /* maximum allowed clock ("DacSpeed") */
    int refclk;                  /* Reference frequency. Most cards use 14.31818 MHz, but IBM RGM RAMDAC's don't...*/
    int mclk;                    /* memory clock (some cards only) */
    char *ck_prog_path;          /* if used: path to ClockProg */
    int clocks[MAX_CLOCKS*2];    /* if used: array of clock values */
} t_clockdef;

#define INIT_CLOCKDEF { 0 , CLKCHIP_NONE , DEFAULT_MAXCLOCK, REFCLK_NOT_DEFINED, MCLK_NOT_DEFINED, NULL, }


/*** external definitions (globals) ***/

extern int addhsync(float min, float max);
extern int addvsync(float min, float max);

extern t_mode *text_mode_list;
extern t_mon_timing *h_mon_limits;
extern t_mon_timing *v_mon_limits;
extern t_clockdef clock_data;
extern t_terminals *p_terminals;
extern t_fontdef font_data;
extern int chipset;
extern char *resetprogpath;
extern char *defaultmode;
extern int underline_pos;
extern int bordercolor;
extern int cursor_start, cursor_end;

extern const char *ConsoleDevice(const char *number);

#endif  


