%{
/*
  SVGATextMode -- An SVGA textmode manipulation/enhancement tool

  Copyright (C) 1995  Koen Gadeyne

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


  Based on earlier work done for svgalib by Stephen Lee.
*/

/****************************************************************************/
/* C DECLARATIONS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg_structs.h"
#include "chipset.h"
#include "validate.h"
#include "messages.h"
#include "misc.h"
#include <sys/stat.h>

/* if VGA_CAN_DO_64KB is defined, 32k chars per screen are allowed */
#ifdef VGA_CAN_DO_64KB
#define MAXCHARS 32*1024
#else
#define MAXCHARS 16*1024
#endif

/* stuff exported by the lexer */
extern int yylex();
extern int line_num;

int addmode(t_mode *mval);
int addhsync(float min, float max);
int addvsync(float min, float max);

void complete_modestruct(t_mode *mtp);
t_mode *add_mode_data(char *name, float clock,
                   int hds, int hss, int hse, int hde,
                   int vds, int vss, int vse, int vde);

char* check_path(char *fname, int line);
void check_fontsize(int x, int y);
void check_hshift(int x);
void check_clock(float clk, char* descr, int line);

#define yyerror(s) PERROR(("%s on line %d in config file\n", (s), line_num));

%}
%union {
    float fval;			/* for returning floating-point values */
    int ival;			/* integer values */
    char *sval;			/* string values (malloc'd) */
    t_mode *mval;		/* mode values (malloc'd) */
}

/****************************************************************************/
/* BISON DECLARATIONS */

%token MODELINE DIM TERMINALS UNDERLINE BORDERCOL
%token CLOCKS DACSPEED MCLK REFCLK CURSOR
%token RESETPROG CLOCKPROG HORIZSYNC VERTREFRESH
%token FONTSELECT

%token <ival> FLAGS UNDERLINE INT
%token <ival> HSYNC VSYNC
%token <fval> FLOAT

%token <sval> QSTRING SMODELINE DFLTMODE
%token <sval> STRING
%token FONT FONTPROG FONTPATH
%token HSHIFT

%token CHIPSET
%token <ival> CHIPSETTYPE CLOCKCHIPTYPE OPTIONDEF

%type <mval> modeline
%type <sval> font_sel
%type <fval> num

%%
lines: /* empty string */
	| lines '\n'
	| lines modeline '\n'			{ addmode($2); complete_modestruct($2); }
	| lines hsyncline '\n'
	| lines vsyncline '\n'
	| lines clocksline '\n'
	| lines cursordef '\n'
	| lines DFLTMODE QSTRING '\n'		{ defaultmode = safe_strdup($3); }
	| lines CLOCKCHIPTYPE '\n'		{ clock_data.clockchiptype = $2; }
	| lines DACSPEED num '\n'		{
	                                            check_clock($3, "DacSpeed", line_num-1);
	                                            clock_data.maxclock = (int)($3*1000);
	                                        }
	| lines CHIPSETTYPE '\n'		{
	                                            chipset = $2;
	                                            /* if max clock not defined yet (DacSpeed line), assign chipset default */
	                                            if (clock_data.maxclock==DEFAULT_MAXCLOCK)
	                                              clock_data.maxclock=ChipsetData[chipset].maxclock;
	                                        }
	| lines MCLK num '\n'			{
	                                            check_clock($3, "MClk", line_num-1);
	                                            clock_data.mclk = (int)($3*1000);
	                                        }
	| lines REFCLK num '\n'			{
	                                            check_clock($3, "RefClk", line_num-1);
	                                            clock_data.refclk = (int)($3*1000);
	                                        }
	| lines OPTIONDEF '\n'			{ OFLG_SET($2); }
	| lines UNDERLINE INT'\n'		{
	                                            if (($3<1) || ($3>32))
	                                              PERROR(("Underline position %d out of bounds [1..32] on line %d in config file\n", $3, line_num-1));
	                                            underline_pos = $3;
	                                        }
	| lines BORDERCOL INT'\n'		{
	                                            if (($3<0) || ($3>255))
	                                              PERROR(("BorderColor %d out of bounds [0..255] on line %d in config file\n", $3, line_num-1));
	                                            bordercolor = $3; }
	| lines RESETPROG QSTRING '\n'		{
	                                            resetprogpath =
	                                              check_path($3,line_num+1);
	                                        }
	| lines CLOCKPROG QSTRING '\n'		{
	                                            clock_data.ck_prog_path =
	                                              check_path($3, line_num+1);
	                                        }
	| lines FONTPROG QSTRING '\n'		{
	                                            font_data.fontprogpath =
	                                              check_path($3, line_num+1);
	                                        }
	| lines FONTPATH QSTRING '\n'		{
	                                            font_data.fontpath =
	                                              check_path($3, line_num+1);
	                                        }
	| lines term_line '\n'
	| lines font_sel '\n'
	| lines QSTRING '\n'                    {
	                                            printf("%s\n",$2);
	                                        }
;

/* included so that float values can also be specified in the form of an INT */
num:	FLOAT
	| INT { $$ = $1; }
;

modeline: MODELINE QSTRING num INT INT INT INT INT INT INT INT
        {
            $$ = add_mode_data($2,$3,$4,$5,$6,$7,$8,$9,$10,$11);
        }
	| SMODELINE num INT INT INT INT INT INT INT INT
	{ 
            $$ = add_mode_data($1,$2,$3,$4,$5,$6,$7,$8,$9,$10);
	}
	| modeline HSYNC
	{
	    $$ = $1;
	    $$->hpol=$2;
	}
	| modeline VSYNC
	{
	    $$ = $1;
	    $$->vpol=$2;
	}
	| modeline FLAGS
	{
	    $$ = $1;
	    MOFLG_SET($$,$2);
	}
	| modeline HSHIFT INT
	{ 
	    $$ = $1;
	    check_hshift($3);
	    $$->hshift = $3;
	}
	| modeline FONT INT DIM INT
	{ 
	    $$ = $1;
	    check_fontsize($3,$5);
	    $$->FontWidth = $3;
	    $$->FontHeight = $5;
	}
;

font_sel: FONTSELECT INT DIM INT
	{
          PERROR(("Syntax error on line %d in config file.\n"\
                  "Possible cause: the syntax of the FontSelect lines has changed!\n"\
                  "See the TextConfig(5) manual file for details.\n", line_num));
	}
	| FONTSELECT QSTRING
	{
	    $$ = safe_strdup($2);
	}
	| font_sel INT DIM INT 
	{
	    $$ = $1;
	    check_fontsize($2, $4);
	    font_data.font_table[$2-8][$4-1] = $1;
	}
;

hsyncline: HORIZSYNC
	| hsyncline num '-' num		{ addhsync($2, $4); }
	| hsyncline num			{ addhsync($2, $2); }
	| hsyncline ',' num '-' num	{ addhsync($3, $5); }
	| hsyncline ',' num		{ addhsync($3, $3); }
;

vsyncline: VERTREFRESH
	| vsyncline num '-' num		{ addvsync($2, $4); }
	| vsyncline num			{ addvsync($2, $2); }
	| vsyncline ',' num '-' num	{ addvsync($3, $5); }
	| vsyncline ',' num		{ addvsync($3, $3); }
;

cursordef: CURSOR
	| cursordef INT '-' INT		{ 
	                                  if ($2<$4)
	                                    { cursor_start = $2; cursor_end = $4;}
	                                  else
	                                    { cursor_start = $4; cursor_end = $2;}
	                                }
	| cursordef INT			{ cursor_start = $2; cursor_end = $2; }
;

clocksline: CLOCKS
	| clocksline num 		{
                                            check_clock($2, "Clock", line_num);
	                                    clock_data.clocks[clock_data.num_clocks++] = (int) ($2*1000);
	                                }
;

term_line: TERMINALS
	| term_line QSTRING 		{
	                                  t_terminals *p_t;
	                                  char *full_path;
	                                  
                                          full_path = safe_malloc(strlen($2)+6);
	                                  strcpy(full_path, "/dev/");
	                                  strcat(full_path, $2);
	                                  check_path(full_path, line_num);

                                          p_t = safe_malloc(sizeof(t_terminals));
                                          p_t->name = full_path;
                                          p_t->next = p_terminals;
                                          p_terminals = p_t;
                                        }
;

%%
/****************************************************************************/
/* ADDITIONAL C CODE */

extern FILE *yyin;

void yy_check_int_range(int cvalue, int lmin, int lmax, char *descstr, char *remark)
{
  if (cvalue<lmin || cvalue>lmax)
    PERROR(("%s = %d out of range [%d..%d] in config file on line %d%s\n",\
             descstr, cvalue, lmin, lmax, line_num, remark ? remark : ""));
}

int addmode(t_mode *mval)
{
    mval->next = text_mode_list;
    text_mode_list = mval;

    return 0;
}

/* fill in some values that are NOT in the mode line, like H/V freq, XxY size, ...*/
void complete_modestruct(t_mode *mtp)
{
    mtp->rows = mtp->VDisplay / mtp->FontHeight;
    mtp->cols = (mtp->HDisplay / 8) & 0xFFFFFFFE; /* must be multiple of 2 in VGA byte-mode adressing */
    mtp->VDisplay = mtp->FontHeight * mtp->rows;  /* make mtp->VDisplay integer multiple of mtp->FontHeight */
    
    if (mtp->rows * mtp->cols > MAXCHARS)
      PERROR(("Maximum of %d characters exceeded (%dx%d = %d) on modeline on line %d\n.", \
               MAXCHARS, mtp->cols, mtp->rows, mtp->cols * mtp->rows));

    if (MOFLG_ISSET(mtp,ATTR_DOUBLESCAN))
    {
      mtp->VDisplay *= 2;
      mtp->VSyncStart *= 2;
      mtp->VSyncEnd *= 2;
      mtp->VTotal *= 2;
    }

    mtp->hfreq = (mtp->pixelClock*1000)/( ((int)(mtp->HTotal / 8)) * mtp->FontWidth);
    mtp->vfreq = (mtp->hfreq*1000)/mtp->VTotal;
    if (MOFLG_ISSET(mtp,ATTR_INTERLACE)) mtp->vfreq *= 2;
}


t_mode *add_mode_data(char *name, float clock,
                   int hds, int hss, int hse, int hde,
                   int vds, int vss, int vse, int vde)
{ 
    t_mode *mtp;

   /*
    * Do some SEVERE error checking on the text mode string timings!
    * the ranges are somewhat randomly chosen. Need to study REAL hardware limits for this...
    */
    yy_check_int_range(hds, 16, 2048, "active pixels", NULL);
    yy_check_int_range(hss, hds, 2048, "start of H-sync", NULL);
    yy_check_int_range(hse, hss+1, hss+(32*8), "end of H-sync", " (Max Hsync width = 256 pixels)");
    yy_check_int_range(hde, hse, 2048, "total pixels", NULL);

    yy_check_int_range(vds, 16, 1023, "active lines", NULL);
    yy_check_int_range(vss, vds, 1023, "start of V-sync", NULL);
    yy_check_int_range(vse, vss+1, vss+16, "end of V-sync", " (Max Vsync width = 16 lines)");
    yy_check_int_range(vde, vse, 1023, "total lines", NULL);

    /* mode has passed initial checks. enter it into the data base */
    mtp = safe_malloc(sizeof(t_mode));

    mtp->name = safe_strdup(name);
      
    mtp->pixelClock = clock * 1000;
    mtp->HDisplay = hds;
    mtp->HSyncStart = hss;
    mtp->HSyncEnd = hse;
    mtp->HTotal = hde;
    mtp->VDisplay = vds;
    mtp->VSyncStart = vss;
    mtp->VSyncEnd = vse;
    mtp->VTotal = vde;

    mtp->flags = 0;        /* default */
    mtp->FontWidth = 8;    /* default */
    mtp->FontHeight = 16;  /* default */

    /* No left shift -- will be overridden by specified hshift on mode line */
    mtp->hshift = 0;

    /* Auto sync polarity -- will be overridden by specified polarities on mode line */
    if (vds < 400)      { mtp->hpol = POS ; mtp->vpol = NEG; }
    else if (vds < 480) { mtp->hpol = NEG ; mtp->vpol = POS; }
    else if (vds < 768) { mtp->hpol = NEG ; mtp->vpol = NEG; }
    else                { mtp->hpol = POS ; mtp->vpol = POS; }

    return mtp;
}


t_mon_timing* add_sync(float min, float max, int max_dev)
{
    t_mon_timing *tim;

    if ((min<5.0) || (min>200.0)) PERROR(("Sync range value %.3f out of bounds on line %d of config file\n", min, line_num));    
    if ((max<5.0) || (max>200.0)) PERROR(("Sync range value %.3f out of bounds on line %d of config file\n", max, line_num));    

    tim = safe_malloc(sizeof(t_mon_timing));

    /* if min==max, allow for a margin on deviation, which is realistic for most monitors */
    if (min==max)
    {
       tim->low_limit = min*1000-max_dev;
       tim->high_limit = max*1000+max_dev;
    }
    else if (min<=max)
    {
       tim->low_limit = min*1000;
       tim->high_limit = max*1000;
    }
    else
    {
       tim->low_limit = max*1000;
       tim->high_limit = min*1000;
    }
    return tim;
}

int addhsync(float min, float max)
{
    t_mon_timing *tim;

    tim = add_sync(min,max,MAX_HSYNC_DEVIATION);
    tim->next = h_mon_limits;
    h_mon_limits = tim;

    return 0;
}

int addvsync(float min, float max)
{
    t_mon_timing *tim;

    tim = add_sync(min,max,MAX_VSYNC_DEVIATION);
    tim->next = v_mon_limits;
    v_mon_limits = tim;

    return 0;
}

char* check_path(char *fname, int line)
{
    struct stat fstatus;
    char *cp;
    
    cp = safe_strdup(fname);
    cp = strtok(cp," ");
    
    if (stat(cp, &fstatus)) {
      PWARNING(("Path `%s' specified on line %d in config file does not exist\n", cp, line));
      return NULL;
    }

    return safe_strdup(fname);
}

void check_fontsize(int x, int y)
{
    if ((x<8) || (x>9)) PERROR(("Illegal font width %d on line %d in config file\n", x, line_num));
    if ((y<1) || (y>32)) PERROR(("Illegal font height %d on line %d in config file\n", y, line_num));
}

void check_hshift(int x)
{
    if ((x<0) || (x>3)) PERROR(("Illegal left shift %d on line %d in config file\n", x, line_num));
}

void check_clock(float clk, char* descr, int line)
{
    if ((clk<MIN_CLOCK) || (clk>MAX_CLOCK))
      PERROR(("%s value %.3f out of bounds on line %d in config file\n", descr, clk, line));
}

