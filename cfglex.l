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
 
#include <stdlib.h>
#include <string.h>
#include "cfg_structs.h"
#include "chipset.h"
#include "y.tab.h"
#include "messages.h"
#include "misc.h"

#define unquote_yytext  ( *(yytext+(yyleng-1))='\0' , (yytext+1) )   /* works, but doesn't allow spaces */
#define unquote1_yytext  ( *(yytext+(yyleng-1))='\0' , (yytext) )   /* works, but doesn't allow spaces */

/* this is a kludge. There should be a way to get rid of the quotes in LEX itself */
#define unquote_strdup_yytext  ( unqstr=safe_strdup(yytext) , \
                                 sscanf(yytext, " \"%s ", unqstr) , \
                                 (*(unqstr+(strlen(unqstr)-1))='\0') , \
                                 unqstr \
                               )
#define yyerror(s) PERROR(("%s on line %d of config file\n", (s), line_num));

int find_token(t_str_token *rec, char *ch_str);
int find_token_sh(t_str_token *rec, char *ch_str);
extern void *safe_strdup(const char *s);

int line_num = 1;
char *unqstr;



%}

/* These options reduce the warnings when compiling with -Wall, but need at least flex 2.5 */
%option noyywrap
%option nounput

delim		[ \t]
ws		{delim}+
letter		[A-Za-z]
int		[0-9]+
float		{int}(\.{int})?([Ee][+\-]?{int})?
comment		\#[^\n]*
qstring		\"[^\"\n]*["\n]
beginline	^{delim}*

%x X_CHIPSET X_CLOCKCHIP X_OPTION X_MODELINE X_FONTSPEC X_QSTR X_FLT X_SYNC
%x X_INT X_INTRANGE X_FONTSELECT

%%


{ws}|{comment} |
<X_CHIPSET>{ws}|{comment} |
<X_CLOCKCHIP>{ws}|{comment} |
<X_OPTION>{ws}|{comment} |
<X_MODELINE>{ws}|{comment} |
<X_QSTR>{ws}|{comment} |
<X_FLT>{ws}|{comment} |
<X_SYNC>{ws}|{comment} |
<X_INTRANGE>{ws}|{comment} |
<X_INT>{ws}|{comment} |
<X_FONTSELECT>{ws}|{comment} |
<X_FONTSPEC>{ws}|{comment}	{ /* ignore blanks and comments */ }

{beginline}clockchip      { BEGIN(X_CLOCKCHIP); }
{beginline}chipset        { BEGIN(X_CHIPSET); }
{beginline}option         { BEGIN(X_OPTION); }
{beginline}modeline       { BEGIN(X_MODELINE); return(MODELINE); }
{beginline}defaultmode    { BEGIN(X_QSTR); return(DFLTMODE); }
{beginline}resetprog      { BEGIN(X_QSTR); return (RESETPROG); }
{beginline}clockprog      { BEGIN(X_QSTR); return (CLOCKPROG); }
{beginline}fontprog       { BEGIN(X_QSTR); return (FONTPROG); }
{beginline}fontpath       { BEGIN(X_QSTR); return (FONTPATH); }
{beginline}terminals      { BEGIN(X_QSTR); return (TERMINALS); }
{beginline}clocks         { BEGIN(X_FLT); return (CLOCKS); }
{beginline}dacspeed       { BEGIN(X_FLT); return (DACSPEED); }
{beginline}mclk           { BEGIN(X_FLT); return (MCLK); }
{beginline}refclk         { BEGIN(X_FLT); return (REFCLK); }
{beginline}horizsync      { BEGIN(X_SYNC); return (HORIZSYNC); }
{beginline}vertrefresh    { BEGIN(X_SYNC); return (VERTREFRESH); }
{beginline}cursor         { BEGIN(X_INTRANGE); return (CURSOR); }
{beginline}underline      { BEGIN(X_INT); return(UNDERLINE); }
{beginline}bordercolor    { BEGIN(X_INT); return(BORDERCOL); }
{beginline}fontselect     { BEGIN(X_FONTSELECT); return(FONTSELECT); }
{beginline}echo           { BEGIN(X_QSTR); }

{beginline}{qstring}      { 
                            BEGIN(X_MODELINE);
                            yylval.sval = unquote_strdup_yytext;
                            return(SMODELINE);
                          }


<X_CHIPSET>{qstring}    { yylval.ival = find_token(ChipsetRec, unquote_yytext); return (CHIPSETTYPE); }

<X_CLOCKCHIP>{qstring}  { yylval.ival = find_token(ClockchipRec, unquote_yytext); return (CLOCKCHIPTYPE); }

<X_OPTION>{qstring}     { yylval.ival = find_token_sh(OptionRec, unquote_yytext); return (OPTIONDEF); }

<X_MODELINE>font	{ return (FONT); }
<X_MODELINE>hshift	{ return (HSHIFT); }
<X_MODELINE>\+hsync	{ yylval.ival = POS ; return (HSYNC); }
<X_MODELINE>\+vsync	{ yylval.ival = POS ; return (VSYNC); }
<X_MODELINE>\-hsync	{ yylval.ival = NEG ; return (HSYNC); }
<X_MODELINE>\-vsync	{ yylval.ival = NEG ; return (VSYNC); }
<X_MODELINE>interlace	{ yylval.ival = ATTR_INTERLACE  ; return (FLAGS); }
<X_MODELINE>doublescan	{ yylval.ival = ATTR_DOUBLESCAN ; return (FLAGS); }
<X_MODELINE>{qstring}	{ yylval.sval = safe_strdup(unquote_yytext); return (QSTRING); }
<X_MODELINE>{int}	{ yylval.ival = atoi(yytext); return (INT); }
<X_MODELINE>{float}	{ yylval.fval = atof(yytext); return (FLOAT); }
<X_MODELINE>[x\*]	{ return (DIM); }

<X_QSTR>{qstring}	{ yylval.sval = safe_strdup(unquote_yytext); return (QSTRING); }

<X_FLT>{float}		{ yylval.fval = atof(yytext); return (FLOAT); }

<X_SYNC>{float}		{ yylval.fval = atof(yytext); return (FLOAT); }
<X_SYNC>[,-]		{ return *yytext; }

<X_INTRANGE>{int}	{ yylval.ival = atoi(yytext); return (INT); }
<X_INTRANGE>[-]		{ return *yytext; }

<X_INT>{int}		{ yylval.ival = atoi(yytext); return (INT); }

<X_FONTSELECT>{qstring}	{ yylval.sval = safe_strdup(unquote_yytext); return (QSTRING); }
<X_FONTSELECT>{int}	{ yylval.ival = atoi(yytext); return (INT); }
<X_FONTSELECT>[x\*]	{ return (DIM); }
<X_FONTSELECT>, 	{ /* eat comma's */ }

\n |
<X_CHIPSET>\n |
<X_CLOCKCHIP>\n |
<X_OPTION>\n |
<X_FONTSPEC>\n |
<X_QSTR>\n |
<X_FLT>\n |
<X_SYNC>\n |
<X_INTRANGE>\n |
<X_INT>\n |
<X_FONTSELECT>\n |
<X_MODELINE>\n	{  BEGIN 0; line_num++; return *yytext; }




. |
<X_CHIPSET>. |
<X_CLOCKCHIP>. |
<X_OPTION>. |
<X_FONTSPEC>. |
<X_QSTR>. |
<X_FLT>. |
<X_SYNC>. |
<X_INTRANGE>. |
<X_INT>. |
<X_FONTSELECT>. |
<X_MODELINE>.	{ yyerror("Unknown token"); }


%%

int find_token(t_str_token *rec, char *ch_str)
{
  int i = 0;
  while (rec[i].name!=ENDREC)
  {
    if (!strcasecmp(rec[i].name_str,ch_str))
    {
      if (i != rec[i].name)
      {
        PERROR(("Internal error in cfglex.l: rec[].name/offset mismatch: i=%d, rec[i].name=%d, configfile line %d\n",\
                 i, rec[i].name, line_num));
      }
      return i;
    }
    i++;
  }
  yyerror("Unknown token");
  return ENDREC;
}

int find_token_sh(t_str_token *rec, char *ch_str)
{
  int i = 0;
  while (rec[i].name!=ENDREC)
  {
    if (!strcasecmp(rec[i].name_str,ch_str))
    {
      if ((1<<i) != rec[i].name)
      {
        PERROR(("Internal error in cfglex.l: rec[].name/offset mismatch: 1<<i=%d, rec[i].name=%d, configfile line %d\n",\
                 1<<i, rec[i].name, line_num));
      }
      return 1<<i;
    }
    i++;
  }
  yyerror("Unknown token");
  return ENDREC;
}

#ifndef yywrap
int yywrap() {return 1;}
#endif
