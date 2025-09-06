/*  user_time() -- a high-resolution system-timer function.
 *
 *  Copyright (C) 1995-1998  Harald Koenig
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

#include "misc.h"
#include "vga_prg.h"
#include "messages.h"

#ifdef DOS
#define CLOCK_TICK_RATE       1193180L /* Underlying HZ */
#define LATCH  65536.0          /* For divider */
#define HZ (CLOCK_TICK_RATE/LATCH)
#else
#define HZ 100
#define CLOCK_TICK_RATE       1193180 /* Underlying HZ */
#define LATCH  ((CLOCK_TICK_RATE + HZ/2) / HZ)        /* For divider */
#endif

unsigned long user_time()
{
   static unsigned long time=0;
   static unsigned short last_count=0;
   unsigned short count = 0;

/* 
   note: dynamic detection of the max. latch value is only possible
   for values below 32768 or frequencies above 36.5 Hz 

   note 2: this will block if counter #0 does not run!
*/

#define DYNAMIC_MAX_LATCH
#ifdef DYNAMIC_MAX_LATCH

   static unsigned short top, itop;
   static int init=1;

   if (init) {
      int i;

      init=0;
      top = itop = 0;
      for(i=0; i<100; i++) {
	 do {
	    last_count = count;
	    outb_p(0x00, 0x43);
	    count = inb_p(0x40);
	    count |= inb(0x40) << 8;
#ifndef DOS
	    if (count & 0x8000) {
	       /* sometimes we're getting out of sync reading the latch :-( */
	       inb(0x40);
	       continue;
	    }
#endif
	 } while (count<last_count);
	 if (count > top) {
	    top = count;
	    itop = i;
	 }
	 else if (i-itop > 50) break;
      }
      PDEBUG(("user_time: refclock %.2f Hz (%d interations %x %u)\n"
            ,(double)(CLOCK_TICK_RATE)/top,itop,top,top));
   }
#endif

   outb(0x00, 0x43);
   count = inb_p(0x40);
   count |= inb(0x40) << 8;
   
   if (count > last_count)   /* timer underrun */
      time += (unsigned long)(1e6*LATCH/CLOCK_TICK_RATE);
   last_count = count;
   return time + (unsigned long)((((1000000.0+HZ/2)/HZ) * (long)(LATCH-(unsigned long)count)) / (long)(LATCH));
}

