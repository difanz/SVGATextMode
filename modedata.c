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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef DOS
#include <unistd.h>
#endif

#include "misc.h"
#include "vga_prg.h"
#include "messages.h"
#include "probe.h"
#include "modedata.h"

#ifdef DOS
#define get_VGA_io_perm(x)
#undef Renounce_SUID
#define Renounce_SUID
extern int optind;
#endif


int roundup2(int num)
 /* round num up to the nearest power of 2 */
{
  int cnt=0;
  while (num)
  {
    num >>=1;
    cnt++;
  }
  return(1<<cnt);
}

bool check2(int *low, int *high, bool *wrap)
{
  if (*high < *low)
    {
      *high += roundup2(*low - *high -1);
      *wrap = TRUE;
      return TRUE;
    }
    return FALSE;
}

int wrapcheck(int *active , int *start_sync, int *stop_sync, int *total,
              int *start_blank, int *end_blank,
              int badsyncmask, int wrapmsgmask)
/* check for wrapping at standard VGA register limits: e.g. 1025 --> 1 */
{
  bool wrap = FALSE;
  int mask = 0;

  check2(active, start_sync, &wrap);
  check2(start_sync, stop_sync, &wrap);
  check2(start_blank, end_blank, &wrap);
     
  if (*total < *stop_sync)
    {
      if (!check2(start_sync, total, &wrap)) mask |= badsyncmask;
    }
    
  if (wrap) mask |= wrapmsgmask;

  return(mask);
}

int wrapcheck_preset(int *active , int *start_sync, int *stop_sync, int *total,
                     int *start_blank, int *end_blank,
                     int wrapmsgmask, int realval)
/* check for more severe/undetected wrapping with given (=known) size */
{
  int offset = 0;
  
  if (realval == *active) return 0;
  if ((realval % 1024) == *active)
  {
    offset = realval - *active;
    *active += offset;
    *start_sync += offset;
    *stop_sync += offset;
    *total += offset;
    return wrapmsgmask;
  }
  else
    /* the preset value doesn't match with measured result. Wait till post_clockprobe_intelligence
     * before storing an error: it could be a special mode
     */
    return 0;
}

#define mul_Vtim(val) do {\
    m->mode_line.VDisplay *= (val);\
    m->mode_line.VSyncStart *= (val);\
    m->mode_line.VSyncEnd *= (val);\
    m->mode_line.VTotal *= (val);\
    m->startvbl *= (val);\
    m->endvbl *= (val);\
    } while (0)

#define div_Vtim(val) do {\
    m->mode_line.VDisplay /= (val);\
    m->mode_line.VSyncStart /= (val);\
    m->mode_line.VSyncEnd /= (val);\
    m->mode_line.VTotal /= (val);\
    m->startvbl /= (val);\
    m->endvbl /= (val);\
    } while (0)


void pre_clockprobe_intelligence(modestruct* m, int xx, int yy)
{
 /* check for "wraparound" of the H and V values: this can be caused by some chipsets using 
  * their own, extended custom registers for setting H/V parameters.
  * A common example of this is the following V-parameters: 1024 5 7 25 instead of
  * 1024 1029 1031 1049
  */
  
  m->remarks |= wrapcheck(&m->mode_line.VDisplay, &m->mode_line.VSyncStart,
                          &m->mode_line.VSyncEnd, &m->mode_line.VTotal,
                          &m->startvbl, &m->endvbl, 
                          MSG_BAD_VSYNC_STOP, MSG_VTIM_MOD);
  
  /* see if vert. resolution was multiplied by 2 (normally for modes with >1024 lines) */

  if (GMOFLG_ISSET(*m,VERT_DOUBLE)) mul_Vtim(2);

  /* now check that really big modes didn't cause a giant wrap that can't be detected by the code above.
   * This could occur e.g. in 1600x12000 modes, which would detect as only 176 (1200-1024) pixels.
   */

  if (yy>0)
  {
    m->remarks |= wrapcheck_preset(&m->mode_line.VDisplay, &m->mode_line.VSyncStart,
                                   &m->mode_line.VSyncEnd, &m->mode_line.VTotal,
                                   &m->startvbl, &m->endvbl, 
                                   MSG_PRESET_VTIM_MOD, yy);
  }

  /* same story for hor. timings */
  
  m->remarks |= wrapcheck(&m->mode_line.HDisplay, &m->mode_line.HSyncStart,
                          &m->mode_line.HSyncEnd, &m->mode_line.HTotal,
                          &m->starthbl, &m->endhbl, 
                          MSG_BAD_HSYNC_STOP, MSG_HTIM_MOD);  
  if (xx>0)
  {
    m->remarks |= wrapcheck_preset(&m->mode_line.HDisplay, &m->mode_line.HSyncStart,
                                   &m->mode_line.HSyncEnd, &m->mode_line.HTotal,
                                   &m->starthbl, &m->endhbl, 
                                   MSG_PRESET_HTIM_MOD, xx);  
  }
  
  /* V-timings could still be wrapped. If so, this will produce extreme X/Y ratios
   * e.g. 1280/2 instead of 1280/1026. This can be arranged by adding 1024 to all V-timings
   * until the ratio is "reasonable"
   */
  
  while ( ((float)m->mode_line.HDisplay / (float)m->mode_line.VDisplay) > 10.0 )
  {
    m->mode_line.VDisplay += 1024;
    m->mode_line.VSyncStart += 1024;
    m->mode_line.VSyncEnd += 1024;
    m->mode_line.VTotal += 1024;
    m->remarks |= MSG_Y_WRAP_COMPENSATE;
  }

/*  
  PDEBUG(("Pre-Clockprobe: %d %d %d %d  %d %d %d %d\n",\
           m->mode_line.HDisplay, m->mode_line.HSyncStart, m->mode_line.HSyncEnd, m->mode_line.HTotal,\
           m->mode_line.VDisplay, m->mode_line.VSyncStart, m->mode_line.VSyncEnd, m->mode_line.VTotal));
*/
}

#define div_Htim(val) do {\
    m->mode_line.HDisplay /= (val);\
    m->mode_line.HSyncStart /= (val);\
    m->mode_line.HSyncEnd /= (val);\
    m->mode_line.HTotal /= (val);\
    m->starthbl /= (val);\
    m->endhbl /= (val);\
    m->mode_line.pixelClock /= (val);\
    } while (0);

#define mul_Htim(val) do {\
    m->mode_line.HDisplay *= (val);\
    m->mode_line.HSyncStart *= (val);\
    m->mode_line.HSyncEnd *= (val);\
    m->mode_line.HTotal *= (val);\
    m->starthbl *= (val);\
    m->endhbl *= (val);\
    m->mode_line.pixelClock *= (val);\
    } while (0);


void post_clockprobe_intelligence(modestruct* m, int xx, int yy)
{
  float ratio;
  int ratiodiv;
  
  /* take double scanning into account. Must be done AFTER clock probe,
   * because it must not be influenced by this.
   * Multiscan not done in text mode, since there the font size _is_ the font size.
   */
  if (GMOFLG_ISSET(*m,(MULTISCAN|DOUBLESCAN|VERT_DOUBLE)))
  {
    /* standard doublescan */
    int div = ( GMOFLG_ISSET(*m,DOUBLESCAN) ? 2 : 1 );
    /* doublescan using font height */
    if (m->txt_gr_mode==MODE_GRAPHICS) div *=  m->mode_line.FontHeight;
    div_Vtim(div);
  }

  /* calculate X/Y ratio */
  
  ratio = (float)m->mode_line.HDisplay / (float)m->mode_line.VDisplay;
  ratiodiv = (int) ((ratio / GOLDEN_RATIO) + 0.5);
  
         /* using a "ratiomul" here could reveal pixmux modes on some cards */

  PDEBUG(("Original X/Y ratio = %1.2f ; ratio divisor = %d\n\n", ratio, ratiodiv));

  /* first see if the pre-set X/Y sizes can help us resolve some problems before we start guessing */
 
  if (xx>0)
  {
    int xratio;
    /* 15/16/24/32 bit modes could cause double/triple/quad H-size */
    xratio = (m->mode_line.HDisplay*100) / xx; /* multiply with 100 allows integer arithmetic */
    switch (xratio)
    {
       case 99:
       case 100:
       case 101: break; /* 1:1 ratio: mode was probed OK. */

       case 199:
       case 200:
       case 201: div_Htim(2);
                 if ((GMOFLG_ISSET(*m,PCS_DIV2)) && (GMOFLG_ISSET(*m,SHIFT_CGA256C)) && (GMOFLG_ISSET(*m,(DOUBLESCAN|MULTISCAN))) && (ratiodiv==2))
                   m->remarks |= MSG_CGA;
                 else
                   m->remarks |= MSG_16BPP;
                 ratiodiv /= 2;
                 break; 

       case 299:
       case 300:
       case 301: div_Htim(3);
                 m->remarks |= MSG_24BPP;
                 ratiodiv /= 3;
                 break; 

       case 399:
       case 400:
       case 401: div_Htim(4);
                 m->remarks |= MSG_32BPP;
                 ratiodiv /= 4;
                 break; 

      /* should we provide the ranges 49-51 etc? for pixmux modes? */
      default: m->remarks |= MSG_PRESET_X_WRONG;
    }
  }
  
  if (yy>0)
  {
    int yratio;
    /* interlaced modes could cause halved V-size */
    yratio = (m->mode_line.VDisplay*100) / yy; /* multiply with 100 allows integer arithmetic */
    switch (yratio)
    {
       case 99:
       case 100:
       case 101: break; /* 1:1 ratio: mode was probed OK. */

       case 49:
       case 50:
       case 51: mul_Vtim(2);
                GMOFLG_SET(*m,INTERLACE);
                m->remarks |= MSG_IS_INTERLACE;
                ratiodiv /= 2;
                break; 

      default: m->remarks |= MSG_PRESET_Y_WRONG;
    }
  }

  /* Could this mode be interlaced ?
   * Trigger on some common V-sizes.
   * This will work only on those particular sizes of course.
   * This code only works for those cards that divide V-timings by 2 for interlacing (i.e. almost all cards)
   */

  if ( (m->txt_gr_mode == MODE_GRAPHICS) && (ratiodiv > 1) )
  {
    if (yy<=0) switch (m->mode_line.VDisplay) /* only when Y-size is not pre-set */
    {
      case 480/2:	/* 640x480 */
      case 600/2:	/* 800x600 */
      case 768/2:	/* 1024x768 */ 
      case 900/2:	/* 1152x900 */
      case 910/2:	/* 1152x910 */
      case 1024/2:	/* 1280x1024 */
      case 1200/2:	/* 1600x1200 */
          if ((ratiodiv % 2) == 0)   /* ratio = 2, 4, ... */
          {
            mul_Vtim(2);
            m->remarks |= MSG_INTERLACE;
            GMOFLG_SET(*m,INTERLACE);
            ratiodiv /=2;
          }
      default:
          break;
    }

   /* after interlacing has been accounted for, check for more trouble */
    
   /* detect 320x200 -like 256 color modes: when this flag is set, the VGA chip outputs data only every 2
    * pixel clocks by assembling two consecutive (4-bit) "pixels" into one (8-bit) pixel. These are the so-called
    * "packed pixel" modes (is this true?).
    * some modes don't have this bit set, and are STILL 320x200. they will need another detection mechanism.
    *
    * There's a problem with this: many VGA cards have an extended register that overrides this setting, so 
    * that, even when the PCS_DIV2 bit is set, the card is STILL working at its full speed, and not the CGA
    * emulation mode. S3 for example has CRTC reg 0x32, where bit 4 enables 256 color modes. Setting the
    * PCS_DIV2 bit doesn't have any effect anymore. Worse even: XFree86 for some odd reason sets the PCS_DIV2
    * flag, although this bit doesn't do what it should do at that moment (XFree86 uses the S3 "256 color
    * enhanced mode" bit just mentionned instead). So that would cause a 640x480 mode on S3 to be detected as 
    * 320x480, since the PCS_DIV2 bit was set (but non-functional). *Sigh*.
    *
    * If you want to remain device-independent, then you're the sucker, since then the PCS_DIV2 is not a certain
    * indication for half-speed 256 color modes anymore...
    */

    if (xx<=0) /* only when X-size was not given */
    {
      if (ratiodiv == 2)
      {
        div_Htim(2); ratiodiv /= 2;
        if ( (GMOFLG_ISSET(*m,PCS_DIV2)) && (GMOFLG_ISSET(*m,SHIFT_CGA256C)) && (GMOFLG_ISSET(*m,(DOUBLESCAN|MULTISCAN))) )
          m->remarks |= MSG_GOLDENRATIO_X_CGA;
        else
          m->remarks |= MSG_GOLDENRATIO_X_HICOLOR;
      }
    }
    if (ratiodiv > 1) m->remarks |= MSG_WEIRD;
  }
}


void getmode(modestruct* m, bool probe_clock, bool raw_mode, int initial_x, int initial_y)

/* get mode parameters. When initial_x and/or initial_y are specified, less guessing is done */

{
  m->remarks=0;

  get_VGA_io_perm(CS_VGA);
  UNLOCK_STD_VGA;

  m->mode_line.HDisplay = Get_HOR_DISPL_END()*8;
  m->mode_line.HSyncStart = Get_HSYNC_START()*8;
  m->mode_line.HSyncEnd = Get_HSYNC_END()*8;
  m->mode_line.HTotal = Get_HOR_TOTAL()*8;
  m->mode_line.VDisplay = Get_VERT_DISPL_END();
  m->mode_line.VSyncStart = Get_VRETRACE_START();
  m->mode_line.VSyncEnd = Get_VRETRACE_END();
  m->mode_line.VTotal = Get_VERT_TOTAL();
     
  m->mode_line.FontWidth = get_charwidth();
  m->mode_line.FontHeight = Get_MAX_SCANLINE();
  m->mode_line.hshift = Get_HORIZ_SHIFT();
     
  m->mode_line.hpol = Get_HSYNC_POLARITY();
  m->mode_line.vpol = Get_VSYNC_POLARITY();
     
  m->starthbl = Get_HBLANK_START()*8;
  m->endhbl = Get_HBLANK_END()*8;
  m->startvbl = Get_VBLANK_START();
  m->endvbl = Get_VBLANK_END();

  m->txt_gr_mode = Get_TX_GR_Mode();

  /* special mode flags */
  m->mode_flags = 0;
  if (Inb_CRTC(0x09) & 0x80) GMOFLG_SET(*m,DOUBLESCAN);
  /* MSL=2 is also used for doublescanning in graphics mode */
  if ((m->txt_gr_mode==MODE_GRAPHICS) && (m->mode_line.FontHeight>1)) GMOFLG_SET(*m,MULTISCAN);
  if (Inb_SEQ(0x01) & 0x08) GMOFLG_SET(*m,CLOCKDIV2);  /* this has already been taken into account by clock probe */
  if (Inb_CRTC(0x17) & 0x40) GMOFLG_SET(*m,BYTEMODE); else GMOFLG_SET(*m,WORDMODE);
  if (Inb_CRTC(0x14) & 0x40) GMOFLG_SET(*m,DOUBLEWORDMODE);
  if (Inb_CRTC(0x17) & 0x04) GMOFLG_SET(*m,VERT_DOUBLE);
  if (inb_ATR_CTL(0x10) & 0x40) GMOFLG_SET(*m,PCS_DIV2);
  if (Inb_CRTC(0x17) & 0x08) GMOFLG_SET(*m,CNT_BY_2);
  if (Inb_GR_CTL(0x05) & 0x20) GMOFLG_SET(*m,SHIFT_CGA4C);
  if (Inb_GR_CTL(0x05) & 0x40) GMOFLG_SET(*m,SHIFT_CGA256C);
  PDEBUG(("Mode flags (mask) = 0x%08x\n", m->mode_flags));
  
  m->logical_width = Inb_CRTC(0x13)*8;
  if (GMOFLG_ISSET(*m,DOUBLEWORDMODE)) m->logical_width *=8;
  else if (GMOFLG_ISSET(*m,WORDMODE)) m->logical_width *=4;
  else m->logical_width *=2;

  /* the "smart" code. This is where the mistakes will be made... */
  if (!raw_mode) pre_clockprobe_intelligence(m, initial_x, initial_y);
  
  if (probe_clock) measure_pixclock(m);

  /* this part of the smart stuff must be AFTER the clock probing, because it would cause wrong results */
  if (!raw_mode) post_clockprobe_intelligence(m, initial_x, initial_y);

}
