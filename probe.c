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
 *** probe.c
 ***
 *** Try to measure CURRENT pixel clock.
 *** Should work on ANY VGA card, since it only uses standard VGA registers
 ***
 *** - No need to disable interrupts to be able to measure!
 *** - Can give slightly inaccurate results on heavily loaded machines (but normally not VERY wrong)
 *** - Due to wraparound in the usec counter (wraps at 1000000 usec), heavily loaded machines could show
 ***   measurement "bins" at values _below_ one time the actual vertical sync interval.
 ***   Bins at a multiple are normal, since a task-switch could make the clock probe miss 
 ***   some (or many) V-interval events. If the probe is switched away for > 1 sec, the usec counter
 ***   has wrapped around, and so an actual value of 1013425 usec between two measurements
 ***   shows up at 13425 usec instead. This is the reason for the "glitches" in the measurements.
 ***
 *** There should even be a possibility to measure H-frequencies using input status 1 bit 0 (0x3DA, bit 0).
 ***
 ***/

#undef DBG_MEASURE 

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#ifndef DOS
#include <unistd.h>
#include <values.h>
#endif
#include <sys/time.h>
#include <sys/types.h>

/* CYGNUS GCC for Windows needs these for gettimeofday() */
#ifdef WIN
extern int gettimeofday(struct timeval *tp, struct timezone *tzp);
#endif

/* if USE_USER_TIME is defined, an alternative reference clock system will
 * be used. This is especially useful on systems which don't have
 * high-precision clocks available (e.g. Some DOS/Windows compilers and some
 * other Unices.
 */

#if (!defined(DOS)) || (!defined(WIN)) && !defined(UCLOCKS_PER_SEC)
#  define UCLOCKS_PER_SEC 1000000L
#endif
#ifdef WIN   /* use Harald Koenig's user_time() code */
#  define USE_USER_TIME
#endif

#ifdef USE_USER_TIME
   extern unsigned long user_time();
#else
#  define user_time nop
#endif


#define URATIO (1000000.0/(float)UCLOCKS_PER_SEC)

#include "misc.h"
#include "vga_prg.h"
#include "file_ops.h"
#include "wait_vsync.h"
#include "messages.h"
#include "modedata.h"


/*
 * The Clock probe. Given the hor. and vert. REAL TOTAL screen size, it returns the pixel clock in MHz
 * 
 * 'REAL' means you have to input the total screen width (which is read from VGA regs in CHARS)
 * multiplied by the font size (8 or 9).
 *
 * NOTE: this function ASSUMES it has the rights to read VGA register STATUS1 !
 *
 * Should work on ANY VGA card, since it only uses standard VGA registers
 *
 * - No need to disable interrupts to be able to measure! (the probe in the X-server does, because it doesn't use timers). 
 * - Can give slightly inaccurate results on heavily loaded machines (but normally not VERY wrong)
 * - Due to unexplained "glitches" in the vertical sync register, some timing attempts go wrong.
 *   this is detected in the program, and it tries again.
 *   Does anyone know WHY those glitches are there, and how to circumvent them?
 * - has the tendency to over-estimate the clock rate by about 0.1 MHz. No real clue why... (or is it just on MY machine?)
 *
 * There should even be a possibility to measure H-frequencies using input status 1 bit 0 (STATUS1, bit 0).
 *
 */
 
#define REALLY_SLOW_IO

/* number of frames to count for timing measurement */
#define NFRAMES    100


/* number sorting function for sort() */
static int compar(long* a, long* b)
{
#ifdef DOS
  if (*a > *b) return 1;
  else  if (*a < *b) return -1;
  else  return 0;
#else
  return (*a - *b);
#endif
}

                   
#define M_RETRY      3            /* maximum number of retries */
#define TIME_BAND    5L           /* in usec, defines the time-band size used for building a histogram */ 
#define VALID_MEASR  NFRAMES*2/3  /* this number of measurements must be valid, or measurement will be bad */


bool measure_pixclock(modestruct* m)

/* This assumes the rest of the mode structure already  contains the correct data
 * This routine only adds the pixel clock and H/V frequencies to the structure
 * (and possibly a remark)
 */

{
#if (!defined(DOS)) || (defined(WIN))
  struct timeval tv;
#endif
  int i;
  long measure[NFRAMES+1];
  volatile long center;  /* "volatile" gets around a bug in GCC 2.7.0 that causes it's value to be lost */
  double scanrate;
  int retries=0;
  double av;
  long current;
  volatile int num, centernum;
  
  int hsize = (m->mode_line.HTotal/8)*m->mode_line.FontWidth;
  int vsize = m->mode_line.VTotal;
  
  int (*compfunc) ();
  compfunc=compar;

#ifdef USE_USER_TIME
  iopl(3);
#endif

  if (!safe_wait_vsync())
  {
    m->mode_line.pixelClock = 0;
    m->mode_line.hfreq = 0;
    m->mode_line.vfreq = 0;
    m->remarks |= MSG_CLOCKPROBE_FAILED;
    return FALSE;
  }

  /* measure */
  do
  {
    /*** Try to do a measurement ***/
    PDEBUG(("Measurement attempt #%d\n",retries));
    waitframe(user_time());    /* synchronize */

    /* this short measurement loop should be optimized heavily, or even written in assembler */
    for (i=0; i<NFRAMES+1; i++)
    {
      waitframe(user_time());    /* measure */
#if (!defined(DOS) || defined(WIN))
#  ifdef USE_USER_TIME
      measure[i] = user_time();
#  else
      gettimeofday(&tv, NULL); 
      measure[i] = tv.tv_usec;  /* this will go wrong if we are task-switched out for > 1 sec ... */
#  endif
#else
      /* DOS DJGPP also has gettimeofday(), but resolution is only 1/18 sec. So we use uclock() */ 
      measure[i] = uclock();
#endif
    }
    /* end of measurement loop */ 

    av = 0;
    /*** convert absolute timer intervals to relative intervals ***/
    for (i=0; i<NFRAMES; i++)
    {
#ifdef DBG_MEASURE
      printf(" %ld", measure[i]);
#endif
      measure[i] = measure[i+1] - measure[i];

      /* UNIX: usec counter wraps around at 1 sec (1000000 usec)
       *
       * DJGPP/DOS: the usec timer does not wrap around at 1 sec.
       *            It is a 64-bit counter, so it NEVER produces a negative difference result
       */
#ifndef DOS
      if (measure[i]<0) measure[i] += 1000000;
#endif
    }
    
#ifdef DBG_MEASURE
      printf("\n");
#endif
    
    /*** sort measurements ***/
    qsort(measure, NFRAMES, sizeof(long), compfunc);

#ifdef DBG_MEASURE
    for (i=0; i<NFRAMES; i++)
      printf(" %ld", measure[i]);
    printf("\n\n");
    fflush(stdout);
#endif

    /*** find value at peak of histogram ***/
    /*** Use that to filter out values out of bounds ***/
    PDEBUG(("URATIO = %1.3f\n", URATIO));
    current = measure[0];
    center = current + TIME_BAND/2;
    centernum = 0;
    i = 0;
    do
    {
      num = 0;
      while ((abs(measure[i]-current) < TIME_BAND) && (i<NFRAMES))
      {
        num++; i++;
      }
      if (num > centernum)
      {
        centernum = num;
        center = current + TIME_BAND/2;
      }
      if (num>0)
      {
        PDEBUG(("Time slot: %1.0f..%1.0f usec, number: %d\n",
                 ((float)current)*URATIO, ((float)current)*URATIO+TIME_BAND, num));
      }
      current += TIME_BAND;
    } while (i<NFRAMES);

    PDEBUG(("Center = %1.0f usec\n", ((float)center)*URATIO));

    /*** Use histogram peak as center value, filter out values out of bounds ***/
    av = 0; num = 0;
    for (i=0; i<NFRAMES; i++)
    {
      if (abs(measure[i]-center) < (TIME_BAND*3))
      {
        av+=measure[i];
        num++;
      }
    }
    av /= num;
    
    retries++;
    PDEBUG(("Measurement: valid measurements: %ld/%d\n", num, NFRAMES));
  }
  while ( (retries<M_RETRY) && (num<VALID_MEASR) );

  scanrate = UCLOCKS_PER_SEC/av;

  PDEBUG(("average framerate (from valid measurements only) = %5.1f usec\n", av * URATIO));
  if (num < VALID_MEASR)
  {
    m->remarks |= MSG_CLOCK_MEASUREMENTS;
    m->valid_measurements = num*100/NFRAMES;
  }

  PDEBUG(("Real total H = %d , total V = %d\n", hsize, vsize));

  m->mode_line.pixelClock = (scanrate * hsize * vsize) / 1000;
  m->mode_line.hfreq = ((m->mode_line.pixelClock*1000)/m->mode_line.HTotal) * 8 / m->mode_line.FontWidth;
  m->mode_line.vfreq = (m->mode_line.hfreq*1000)/m->mode_line.VTotal;
  return TRUE;
}
