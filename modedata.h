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
 *** modedata: routines for text/graphics mode grabbing
 ***/

#ifndef _MODEDATA_H
#define _MODEDATA_H

#include "cfg_structs.h"

/* defines for text/graphics mode */ 
#define MODE_UNDEFINED -1
#define MODE_TEXT      0
#define MODE_GRAPHICS  1

/* golden ratio limits: used for "smart" mode detection code -- they must NOT overlap! */
#define GOLDEN_RATIO   1.333

/* defines for special mode flags */
#define DOUBLESCAN     1<<0       /* doublescan bit on */
#define CLOCKDIV2      1<<1       /* pixel clock is divided by 2 */
#define MULTISCAN      1<<2       /* MSL>2 in graphics mode, used as extended DoubleScan */
#define BYTEMODE       1<<3       /* byte mode memory adressing */
#define WORDMODE       1<<4       /* word mode memory adressing */
#define DOUBLEWORDMODE 1<<5       /* ... */
#define VERT_DOUBLE    1<<6       /* vertical timings are doubled from the ones in the vert. timings regs */
#define PCS_DIV2       1<<7       /* pixel data changes only every 2 pixel clock beats (256 color modes) */
#define CNT_BY_2       1<<8       /* memory address counter increments only every 2nd pixel clock beat */
#define INTERLACE      1<<9       /* interlacing enabled */
#define SHIFT_CGA4C    1<<10      /* CGA 4-color 320x200 shift mode */
#define SHIFT_CGA256C  1<<11      /* CGA 256 color 320x200 shift mode */


/* special remarks concerning the grabbed mode */
typedef struct {
      int msg_mask;
      char* msg_txt;
} msg_str;

enum msg_type { MSG_VTIM_MOD                = 1<<0,
                MSG_HTIM_MOD                = 1<<1,
                MSG_GOLDENRATIO_X_CGA       = 1<<2,
                MSG_GOLDENRATIO_X_HICOLOR   = 1<<3,
                MSG_GOLDENRATIO_Y_INTERLACE = 1<<4,
                MSG_CLOCK_MEASUREMENTS      = 1<<5,
                MSG_INTERLACE               = 1<<6,
                MSG_BAD_HSYNC_STOP          = 1<<7,
                MSG_BAD_VSYNC_STOP          = 1<<8,
                MSG_PRESET_VTIM_MOD         = 1<<9,
                MSG_PRESET_HTIM_MOD         = 1<<10,
                MSG_PRESET_X_WRONG          = 1<<11,
                MSG_PRESET_Y_WRONG          = 1<<12,
                MSG_16BPP                   = 1<<13,
                MSG_24BPP                   = 1<<14,
                MSG_32BPP                   = 1<<15,
                MSG_IS_INTERLACE            = 1<<16,
                MSG_CGA                     = 1<<17,
                MSG_WEIRD                   = 1<<18,
                MSG_Y_WRAP_COMPENSATE       = 1<<19,
                MSG_CLOCKPROBE_FAILED       = 1<<20 };

typedef struct modestruct {
      t_mode mode_line;                            /* basic data which would appear in a mode line */
      int txt_gr_mode;                             /* text or graphics mode: MODE_TEXT or MODE_GRAPHICS */
      int logical_width;                           /* screen width according to offset register */
      int starthbl, endhbl, startvbl, endvbl;      /* blanking parameters */
      int remarks;                                 /* special remarks concerning the grabbed mode */
      int mode_flags;                              /* special mode flags -- should be merged with flags in mode_line */
      int valid_measurements;                      /* percentage of valid measurements */
} modestruct;

#define GMOFLG_SET(m,opt)    ((m).mode_flags |= (opt))
#define GMOFLG_ISSET(m,opt)  ( ((m).mode_flags & (opt)) != 0 )

void getmode(modestruct* m, bool probe_clock, bool raw_mode, int initial_x, int initial_y);

#endif

