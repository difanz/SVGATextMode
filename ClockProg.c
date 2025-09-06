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
 *** ClockProg: Works like SVGATextMode, but programs only pixel and memory clock.
 ***            Could maybe be used as an XFree86 ClockProg, because it enables
 ***            the low clocks needed for 320x200 modes in X, which X seems to
 ***            deny, despite the "divide-by-2" mode that all VGA's have.
 ***/

#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define CHIPSETREC 1  /* included chipset definition struct */
#include "chipset.h"
#include "vga_prg.h" 
#include "unlock_svga.h" 
#include "special_svga.h" 
#include "setclock.h"
#include "file_ops.h"
#include "string_ops.h"
#include "messages.h"
#include "chipset.h"
#include "validate.h"
#include "run_extprog.h"
#include "dump_cfgdata.h"

/*
 * this adds all the global variables that hold the data from the config file.
 */
#include "cfg_data.h"

/*
 * other global variables
 */

char *CommandName;
#ifndef DOS
  char *ConfigFile=CLOCK_CONF_FILE;
  char *ConfigFile2=CONFIGFILE; 
#else
  char *ConfigFile="STMClock.cfg";
  char *ConfigFile2="STMText.cfg";
#endif
bool debug_messages=FALSE;
int debug_level=0;

/*
 * yacc parser functions
 */

extern int yyparse(void);
extern FILE *yyin;
#define param_file yyin

#undef USE_INDEX  /* define if specifying the index is required instead of optional (and ignored) */

void usage()
{
     PMESSAGE(("version %s. (c) 1995-1998 Koen Gadeyne.\n"\
     "  Usage: %s [options] <Frequency> [<Index>]\n"\
     "  Options: -n  Don't program VGA hardware\n"\
     "               (don't do actual pixel clock programming)\n"\
     "           -d  print debugging information\n"\
     "           -h  print usage information\n"\
     "           -m <Frequency>\n"\
     "               Program memory clock with given frequency\n"\
     "               (only works on some clockchips)\n"\
     "           -t <ConfigFile>\n"\
     "               Use <ConfigFile> instead of the default.\n"\
     "               (default = %s if that exists, or %s otherwise)\n"\
     "  <Frequency>: A pixel clock frequency in MHz\n"\
     "  <Index>: The index in the list of available pixel clocks\n"\
     "           Allowed for compatibility with XFree86, but ignored.\n"\
     "           clock specification in config file is used, not this index\n",\
     VERSION, CommandName, ConfigFile, ConfigFile2));
}

/****************************************************************************************************************************/

int main (int argc, char* argv[])
{
  bool program_hardware=TRUE;
  int freq, mclk=MCLK_NOT_DEFINED;
  int c, charwidth;
#ifdef USE_INDEX
  int index;
#endif
    
 /*
  * command-line argument parsing
  */

  CommandName = argv[0];

  while ((c = getopt (argc, argv, "ndhm:t:")) != EOF)
    switch (c)
    {
      case 'n': program_hardware=FALSE;
                break;
      case 'd': debug_level++;
                debug_messages=TRUE;
                break;
      case 'h': usage();
                exit(0);
                break;
      case 't': ConfigFile=safe_strdup(optarg);
                break;
      case 'm': mclk=getfloat(optarg, "memory clock frequency", 0, 100)*1000;
                PDEBUG(("MClk on command line = %d khz\n", mclk));
                break;
      case '?': usage();
                PERROR(("Bad option '-%c'\n",(char)optopt));
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }
   
  PVERSION;
  PDEBUG(("Debug level: %d\n", debug_level));
 
#ifdef USE_INDEX
  if (argc<optind+2)
#else
  if (argc<optind+1)
#endif
  {
    usage();
    PERROR(("Not enough parameters on command line.\n"));
  }

  /* first argument: clock freq in MHz */
  freq = getfloat(argv[optind], "clock frequency", 1, 500)*1000;
  
#ifdef USE_INDEX
  /* second argument: clock index */
  optind++;
  index = getint(argv[optind], "clock index", 0, 127);
#endif

 /*
  * check if default config file exists. If not, use TextConfig.
  */
  
  if ((param_file=fopen(ConfigFile,"r"))==NULL)
  {
    PDEBUG(("`%s' not found, using `%s'\n", ConfigFile, ConfigFile2));
    ConfigFile=safe_strdup(ConfigFile2);
  }
  else
  {
    fclose(param_file);
    PDEBUG(("Using `%s' as config file\n", ConfigFile));
  }

 /*
  * open parameter file, parse the sucker, close up again.
  */
  param_file = open_param_file(ConfigFile);
  PDEBUG(("Parsing Config file...\n"));
  while (!feof(yyin)) { yyparse(); }
  fclose(param_file);

 /*
  * Do some sanity checks
  */
  
  sanitize_cfgfile_data();

 /*
  * show all parsed data
  */

  if (debug_level>0) dump_cfgdata(debug_level);

 /*
  * 3rd argument: memory clock
  */

  optind++;
  if (argc > optind)
  {
    if (clock_data.mclk != MCLK_NOT_DEFINED)
      PDEBUG(("Overriding MClk in config file by value on command line: %d kHz\n", mclk));
    clock_data.mclk = mclk;
  }

 /*
  * See if this setup can generate that clock, and abort of not.
  */
  
  check_clockgen(freq, TRUE);

 /*
  * finally do it... maybe
  */
  
  if (program_hardware)
  {
    get_VGA_io_perm(chipset);

    /* sync disks if requested. Is there any way to do this and be SURE _all_ data has been flushed? */
    if (OFLG_ISSET(OPT_SYNC))
    {
      PMESSAGE(("Syncing disks...\n"));
      sync(); sleep(2); /* same as in "reboot" command. Hopefully enough */
    }

    unlock(chipset);

    PMESSAGE(("Setting Pixel Clock to %1.2f MHz.\n", freq/1000.0));    
    charwidth = ((Inb_SEQ(1) & 1) == 1) ? 8 : 9;
    do_clock(chipset, freq, charwidth);

    special(chipset); /* change chipset-specific things, if needed */

  }
  return(0);
}

