/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/S3gendac.h,v 3.8 1996/10/03 08:34:25 dawes Exp $ */ 


/* Jon Tombs <jon@esix2.us.es>  */


/* $XConsortium: S3gendac.h /main/5 1995/11/12 19:30:20 kaleb $ */

/* 22/10/99 - applied patch from Stanislav V. Voronyi" <stas@esc.kharkov.com>
 *            for S3 Trio 3D2X support.  (Ron)
 */

#define GENDAC_INDEX	     0x3C8
#define GENDAC_DATA	     0x3C9
#define BASE_FREQ ((clock_data.refclk==REFCLK_NOT_DEFINED) ? 14.31818 : clock_data.refclk/1000.0)   /* MHz */

int S3gendacSetClock( 
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

int ET4000gendacSetClock( 
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

int ET4000gendacSetpixmuxClock( 
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

int ET6000SetClock( 
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

int ICS5342SetClock( 
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

/* [kmg] */
int ARK2000gendacSetClock(
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

int S3TrioSetClock( 
#if NeedFunctionPrototypes
   long freq, int clock
#endif
);     

int commonCalcClock(
#if NeedFunctionPrototypes
   long freq,
   int min_m, int min_n, int min_n2, int max_n2,
   long freq_min, long freq_max,
   unsigned char *mdiv, unsigned char *ndiv
#endif
);

int commonSetClock(
#if NeedFunctionPrototypes
   long freq, int clock,
   int min_m, int min_n, int min_n2, int max_n2, int pll_type,
   long freq_min, long freq_max
#endif
);



