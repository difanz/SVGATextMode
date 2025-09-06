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
 *** chipset parameters for SVGATextMode
 ***
 ***/

#ifndef _CHIPSET_H
#define _CHIPSET_H

#include "cfg_structs.h"

extern int STM_Options;
#define OFLG_SET(opt)  (STM_Options |= (opt))
#define OFLG_CLR(opt)  (STM_Options &= ~(opt))
#define OFLG_ISSET(opt)  ( (STM_Options & (opt)) != 0 )

typedef const struct {
  const int name;
  const int clkchip_mask;
  const int optmask;
  const int maxclock;  /* in khz */
} t_chipset_data;

typedef const struct {
  const char *name_str;
  const int name;
} t_str_token;

typedef const struct {
  const int name;
  const int minclock;
  const int maxclock;
} t_clockchip_data;

/*
 * Supported Chipsets
 */

#define CS_NONE       -1

#define CS_VGA        0
#define CS_S3         1
#define CS_CIRRUS     2
#define CS_ET4000     3
#define CS_TVGA8900   4
#define CS_TVGA9000   5
#define CS_PVGA1      6
#define CS_WDC90C0X   7
#define CS_WDC90C1X   8
#define CS_WDC90C2X   9
#define CS_WDC90C3X   10
#define CS_ATI        11
#define CS_ATIMACH32  12
#define CS_VIDEO7     13
#define CS_ALI        14
#define CS_AL2101     15
#define CS_OTI67      16
#define CS_OTI77      17
#define CS_OTI87      18
#define CS_SIS        19
#define CS_REALTEK    20
#define CS_ARK        21
#define CS_NCR22E     22
#define CS_NCR32      23
#define CS_GVGA       24
#define CS_MX         25
#define CS_ET3000     26
#define CS_ET6000     27
#define CS_TGUI       28
#define CS_ATIMACH64  29
#define CS_MATROX     30
#define CS_NEOMAGIC   31
#define CS_RIVA128    32

#define CLKCHIP_NONE        -1

#define CLKCHIP_ICD2061A     0
#define CLKCHIP_ICS9161A     1
#define CLKCHIP_DCS2834      2
#define CLKCHIP_SC11412      3
#define CLKCHIP_S3GENDAC     4
#define CLKCHIP_S3_SDAC      5
#define CLKCHIP_TI3025       6
#define CLKCHIP_ICS2595      7
#define CLKCHIP_ICS5300      8
#define CLKCHIP_ICS5342      9
#define CLKCHIP_CH8391       10
#define CLKCHIP_S3TRIO       11
#define CLKCHIP_CIRRUS       12
#define CLKCHIP_ICS5341      13
#define CLKCHIP_STG1703      14
#define CLKCHIP_TI3026       15
#define CLKCHIP_IBMRGB5XX    16
#define CLKCHIP_ET6000       17
#define CLKCHIP_S3VIRGE      18
/* do not touch the order of the following 3 defines! */
#define CLKCHIP_TGUI9320     19
#define CLKCHIP_TGUI9440     20
#define CLKCHIP_CYBER938x    21
#define CLKCHIP_MACH64       22
#define CLKCHIP_ATT20C408    23
#define CLKCHIP_MGA1064      24
#define CLKCHIP_LAGUNA       25
#define CLKCHIP_RIVA128      26
#define CLKCHIP_MGAG200      27

#define OPT_HIBIT_LOW      1<<0
#define OPT_HIBIT_HIGH     1<<1
#define OPT_ET4000_ALTCLK  1<<2
#define OPT_SWAP_HIBIT     1<<3
#define OPT_LEGEND         1<<4
#define OPT_FAST_DRAM      1<<5
#define OPT_MED_DRAM       1<<6
#define OPT_SLOW_DRAM      1<<7
#define OPT_XFAST_DRAM     1<<8
#define OPT_LOADFONT       1<<9
#define OPT_CLOCKDIV2      1<<10
#define OPT_SPEA_MERCURY   1<<11
#define OPT_SYNC           1<<12
#define OPT_S3_HS_TEXT     1<<13
#define OPT_CLKCHIP_X      1<<14
#define OPT_SOG            1<<15
#define OPT_16COLOR        1<<16
#define OPT_ISO_FONT9      1<<17
#define OPT_TOPLEFT        1<<18

#define ENDREC -1 


#  ifndef CHIPSETREC
extern t_str_token ChipsetRec[];
extern t_chipset_data ChipsetData[];
extern t_str_token ClockchipRec[];
extern t_str_token OptionRec[];
extern t_clockchip_data ClockchipData[];

#  else

#define COMMON_OPTS         OPT_SYNC | OPT_LOADFONT | OPT_16COLOR | OPT_ISO_FONT9
#define DRAM_SPEED_OPTS     OPT_FAST_DRAM | OPT_MED_DRAM | OPT_SLOW_DRAM | OPT_XFAST_DRAM


/* the CS_XXX values will be used later to do direct indexing in this array
 * So we MUST make sure that consistency is maintained when adding chipsets
 * or changing stuff in the array.
 */


t_str_token ChipsetRec[] = {
  { "VGA",       CS_VGA       },
  { "S3",        CS_S3        },
  { "CIRRUS",    CS_CIRRUS    },
  { "ET4000",    CS_ET4000    },
  { "TVGA8900",  CS_TVGA8900  },
  { "TVGA9000",  CS_TVGA9000  },
  { "PVGA1",     CS_PVGA1     },
  { "WDC90C0X",  CS_WDC90C0X  },
  { "WDC90C1X",  CS_WDC90C1X  },
  { "WDC90C2X",  CS_WDC90C2X  },
  { "WDC90C3X",  CS_WDC90C3X  },
  { "ATI",       CS_ATI       },
  { "ATIMACH32", CS_ATIMACH32 },
  { "VIDEO7",    CS_VIDEO7    },
  { "ALI",       CS_ALI       },
  { "AL2101",    CS_AL2101    },
  { "OTI67",     CS_OTI67     },
  { "OTI77",     CS_OTI77     },
  { "OTI87",     CS_OTI87     },
  { "SIS",       CS_SIS       },
  { "REALTEK",   CS_REALTEK   },
  { "ARK",       CS_ARK       },
  { "NCR77C22E", CS_NCR22E    },
  { "NCR77C32",  CS_NCR32     },
  { "GVGA",      CS_GVGA      },
  { "MX",        CS_MX        },
  { "ET3000",    CS_ET3000    },
  { "ET6000",    CS_ET6000    },
  { "TGUI",      CS_TGUI      },
  { "ATIMACH64", CS_ATIMACH64 },
  { "MATROX",    CS_MATROX    },
  { "NEOMAGIC",  CS_NEOMAGIC  },
  { "RIVA128",   CS_RIVA128   },
  { "",          ENDREC       }
};

t_chipset_data ChipsetData[] = {

  { CS_VGA,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    45000
  },

  { CS_S3,
    1<<CLKCHIP_ICD2061A | 1<<CLKCHIP_ICS9161A | 1<<CLKCHIP_DCS2834 | 1<<CLKCHIP_SC11412\
      | 1<<CLKCHIP_S3GENDAC | 1<<CLKCHIP_S3_SDAC | 1<<CLKCHIP_TI3025 | 1<<CLKCHIP_ICS2595\
      | 1<<CLKCHIP_ICS5300 | 1<<CLKCHIP_ICS5342 | 1<<CLKCHIP_CH8391 | 1<<CLKCHIP_S3TRIO\
      | 1<<CLKCHIP_STG1703 | 1<<CLKCHIP_TI3026 | 1<<CLKCHIP_IBMRGB5XX | 1<<CLKCHIP_S3VIRGE,
    COMMON_OPTS | OPT_CLOCKDIV2 | OPT_LEGEND | OPT_SPEA_MERCURY | DRAM_SPEED_OPTS | OPT_S3_HS_TEXT | OPT_SOG,
    70000
  },

  { CS_CIRRUS,
    1<<CLKCHIP_CIRRUS | 1<<CLKCHIP_LAGUNA,
    COMMON_OPTS | DRAM_SPEED_OPTS,
    55000
  },

  { CS_ET4000,
    1<<CLKCHIP_ICS5341 | 1 << CLKCHIP_ICD2061A,
    COMMON_OPTS | OPT_CLOCKDIV2 | OPT_HIBIT_LOW | OPT_HIBIT_HIGH | OPT_ET4000_ALTCLK | OPT_LEGEND | OPT_CLKCHIP_X,
    90000
  },
  
  { CS_TVGA8900,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    50000
  },
  
  { CS_TVGA9000,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    45000
  },
  
  { CS_PVGA1,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    45000
  },
  
  { CS_WDC90C0X,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    50000
  },
  
  { CS_WDC90C1X,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2 | OPT_SWAP_HIBIT,
    50000
  },
  
  { CS_WDC90C2X,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2 | OPT_SWAP_HIBIT,
    50000
  },
  
  { CS_WDC90C3X,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2 | OPT_SWAP_HIBIT,
    50000
  },
  
  { CS_ATI,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    55000
  },
  
  { CS_ATIMACH32,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    55000
  },
  
  { CS_VIDEO7,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    45000
  },
  
  { CS_ALI,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    70000
  },
  
  { CS_AL2101,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000
  },
  
  { CS_OTI67,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000
  },
  
  { CS_OTI77,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000
  },
  
  { CS_OTI87,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    70000
  },
  
  { CS_SIS,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000  /* preliminary: untested */
  },
  
  { CS_REALTEK,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    50000  /* preliminary: untested */
  },
  
  { CS_ARK,
    1<<CLKCHIP_ICS5342,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000  /* preliminary: untested */
  },
  
  { CS_NCR22E,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    50000  /* preliminary: untested */
  },
  
  { CS_NCR32,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000  /* preliminary: untested */
  },
  
  { CS_GVGA,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    50000  /* preliminary: untested */
  },
  
  { CS_MX,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    50000  /* preliminary: untested */
  },
  
  { CS_ET3000,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000  /* preliminary: untested */
  },
  
  { CS_ET6000,
    1<<CLKCHIP_ET6000,
    COMMON_OPTS | OPT_CLOCKDIV2,
    60000
  },
  
  { CS_TGUI,
    1<<CLKCHIP_TGUI9320 | 1<<CLKCHIP_TGUI9440 | 1<<CLKCHIP_CYBER938x,
    COMMON_OPTS | OPT_CLOCKDIV2 | DRAM_SPEED_OPTS,
    45000
  },

  { CS_ATIMACH64,
    1<<CLKCHIP_ICS2595 | 1<<CLKCHIP_STG1703 | 1<<CLKCHIP_MACH64 \
    | 1<<CLKCHIP_ATT20C408 | 1<<CLKCHIP_IBMRGB5XX,
    COMMON_OPTS,
    50000
  },

  { CS_MATROX,
    1<<CLKCHIP_TI3026 | 1<<CLKCHIP_MGA1064 | 1<<CLKCHIP_MGAG200,
    COMMON_OPTS | OPT_SOG,
    200000
  },

  { CS_NEOMAGIC,
    0,
    COMMON_OPTS | OPT_CLOCKDIV2 | OPT_TOPLEFT,
    45000   /* unknown */
  },

  { CS_RIVA128,
    1<<CLKCHIP_RIVA128,
    COMMON_OPTS | OPT_CLOCKDIV2,
    80000
  },

  { CS_NONE,   /* CS_NONE signals the end of the chipset structure */
    0,
    0,
    0
  }
};

/*
 * ClockChips
 */

t_str_token ClockchipRec[] = {
  { "icd2061a",   CLKCHIP_ICD2061A   },
  { "ics9161a",   CLKCHIP_ICS9161A   },
  { "dcs2834",    CLKCHIP_DCS2834    },
  { "sc11412",    CLKCHIP_SC11412    },
  { "s3gendac",   CLKCHIP_S3GENDAC   },
  { "s3_sdac",    CLKCHIP_S3_SDAC    },
  { "ti3025",     CLKCHIP_TI3025     },
  { "ics2595",    CLKCHIP_ICS2595    },
  { "ics5300",    CLKCHIP_ICS5300    },
  { "ics5342",    CLKCHIP_ICS5342    },
  { "ch8391",     CLKCHIP_CH8391     },
  { "S3Trio",     CLKCHIP_S3TRIO     },
  { "Cirrus",     CLKCHIP_CIRRUS     },
  { "ics5341",    CLKCHIP_ICS5341    },
  { "stg1703",    CLKCHIP_STG1703    },
  { "ti3026",     CLKCHIP_TI3026     },
  { "ibm_rgb5xx", CLKCHIP_IBMRGB5XX  },
  { "et6000",     CLKCHIP_ET6000     },
  { "s3virge",    CLKCHIP_S3VIRGE    },
  { "tgui9320",   CLKCHIP_TGUI9320   },
  { "tgui9440",   CLKCHIP_TGUI9440   },
  { "cyber938x",  CLKCHIP_CYBER938x  },
  { "mach64",     CLKCHIP_MACH64     },
  { "att20c408",  CLKCHIP_ATT20C408  },
  { "mystique",   CLKCHIP_MGA1064    },
  { "laguna",     CLKCHIP_LAGUNA     },
  { "riva128",    CLKCHIP_RIVA128    },
  { "mgag200",    CLKCHIP_MGAG200    },
  { "",           ENDREC             }
};

t_clockchip_data ClockchipData[] = {
  { CLKCHIP_ICD2061A,  300   , 110000 },  /* data book says 120 MHz max clock, but that doesn't always work */
  { CLKCHIP_ICS9161A,  300   , 110000 },
  { CLKCHIP_DCS2834,   300   , 110000 },
  { CLKCHIP_SC11412,   12500 , 110000 },  /* needs to be checked. This is just a guess */
  { CLKCHIP_S3GENDAC,  12500 , 125000 },
  { CLKCHIP_S3_SDAC,   12500 , 125000 },
  { CLKCHIP_TI3025,    20000 , 220000 },
  { CLKCHIP_ICS2595,   300   , 110000 },
  { CLKCHIP_ICS5300,   12500 , 125000 },
  { CLKCHIP_ICS5342,   12500 , 125000 },
  { CLKCHIP_CH8391,    8500  , 135000 },
  { CLKCHIP_S3TRIO,    16875 , 135000 },
  { CLKCHIP_CIRRUS,    14318 , 110000 },
  { CLKCHIP_ICS5341,   12500 , 135000 },
  { CLKCHIP_STG1703,   8500  , 135000 },
  { CLKCHIP_TI3026,    13750 , 220000 },
  { CLKCHIP_IBMRGB5XX, 16250 , 220000 },
  { CLKCHIP_ET6000,    12500 , 135000 },
  { CLKCHIP_S3VIRGE,   16875 , 135000 },
  { CLKCHIP_TGUI9320,   8000 , 108000 },  /* needs to be checked. This is just a guess */
  { CLKCHIP_TGUI9440,   8000 , 108000 },
  { CLKCHIP_CYBER938x,  8000 , 108000 },  /* needs to be checked. This is just a guess */
  { CLKCHIP_MACH64,    12500 , 135000 },
  { CLKCHIP_ATT20C408, 12500 , 135000 },  /* min freq is just a guess */
  { CLKCHIP_MGA1064,   12500 , 175000 },  /* min freq is just a guess */
  { CLKCHIP_LAGUNA,    14318 , 110000 },
  { CLKCHIP_RIVA128,   14318 , 110000 },  /* another blunt guess */
  { CLKCHIP_MGAG200,   27051 , 220000 },
  { ENDREC,            0     , 0      }
};

/*
 * Supported Options
 */

int STM_Options = 0;

t_str_token OptionRec[] = {
  { "HIBIT_LOW",          OPT_HIBIT_LOW     },
  { "HIBIT_HIGH",         OPT_HIBIT_HIGH    },
  { "ET4000_AltClockSel", OPT_ET4000_ALTCLK },
  { "SWAP_HIBIT",         OPT_SWAP_HIBIT    },
  { "Legend",             OPT_LEGEND        },
  { "Fast_DRAM",          OPT_FAST_DRAM     },
  { "Med_DRAM",           OPT_MED_DRAM      },
  { "Slow_DRAM",          OPT_SLOW_DRAM     },
  { "XFast_DRAM",         OPT_XFAST_DRAM    },
  { "LoadFont",           OPT_LOADFONT      },
  { "ClockDiv2",          OPT_CLOCKDIV2     },
  { "SPEA_Mercury",       OPT_SPEA_MERCURY  },
  { "SyncDisks",          OPT_SYNC          },
  { "S3_HSText",          OPT_S3_HS_TEXT    },
  { "clockchip_X",        OPT_CLKCHIP_X     },
  { "sync_on_green",      OPT_SOG           },
  { "16color",            OPT_16COLOR       },
  { "iso_font9",          OPT_ISO_FONT9     },
  { "topleft",            OPT_TOPLEFT       },
  { "",                   ENDREC            }
};


#  endif

#endif  
