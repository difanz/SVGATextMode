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

/* THIS PROGRAM IS UNDER CONSTRUCTION --- NOT FUNCTIONAL YET */

/***
 *** probe_clocks: A standalone replacement for the XFREE86 "X -probeonly" command.
 ***               Probes all available pixel clocks using the mechanism specified
 ***               In the /etc/ClockConfig or the /etc/TextConfig file.
 ***               This doesn't need XFree86 to be installed.
 ***
 ***               Since the whole STM package has no chipset detection code in it,
 ***               you have to define the chipset AND the clocks in the config file.
 ***               The clock VALUES may be wrong, but the probe will only probe for
 ***               those that are mentionned in the config file.
 ***
 ***               This is not very nice. We need to use the XFree86 "save" functions
 ***               to save register contents before we go on, and then restore them
 ***               afterwards.
 ***/

#include "misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "vga_prg.h"
#include "setclock.h"
#include "validate.h"
#include "messages.h"

char *CommandName;
char ConfigFile[1024]=CLOCK_CONF_FILE;
bool debug_messages=FALSE;

void usage()
{
     PMESSAGE(("version %s. (c) 1995,1996,1997 Koen Gadeyne.\n Usage: %s [options]\n\n\
     Options: -n  Don't program VGA hardware (rather useless)\n\
              -d  print debugging information\n\
              -h  print usage information\n\
              -f  fast mode: probe only one clock `precisely', and the rest faster
                  and less accurate (XFree86 method for all but the first probe)
              -t <ConfigFile>  Use alternative config file instead of the default\n\
                 (= %s if that exists, or %s otherwise)\n\n",
     VERSION, CommandName, CLOCK_CONF_FILE, CONFIGFILE));
}

void Xfree_probe_clocks()
/* this is largely the code from the XFree86 file XFree86-3.1.1-1/xc/programs/Xserver/hw/xfree86/common_hw/xf86_ClkPr.c */
{
}

/****************************************************************************************************************************/

int main (int argc, char* argv[])
{
  bool program_hardware=TRUE;
  bool fast_mode=FALSE;
  int chipset=CS_NONE;
  const char *str_chipsets[NUM_CHIPSETS] = CHIPSET_STRINGS;
  FILE *param_file;
  int optionmask;
  int n;
  t_clockdef ck;
  char c;
  modestruct m, probe_m;
  char tempstr[1024]="";
    
 /*
  * check if default config file exists. If not, use TextConfig.
  */
  
  if ((param_file=fopen(ConfigFile,"r"))==NULL) strcpy(ConfigFile, CONFIGFILE);
  else fclose(param_file);

 /*
  * command-line argument parsing
  */

  CommandName = argv[0];

  while ((c = getopt (argc, argv, "ndfht:")) != EOF)
    switch (c)
    {
      case 'n': program_hardware=FALSE;
                break;
      case 'd': debug_messages=TRUE;
                break;
      case 'f': fast_mode=TRUE;
                break;
      case 'h': usage();
                exit(0);
                break;
      case 't': strcpy(ConfigFile, optarg);
                break;
      case '?': usage();
                PERROR(("Bad option '-%c'\n",(char)optopt));
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }
   
  PVERSION;

  if (argc>optind+1)
  {
    usage();
    PERROR(("Too many parameters on command line.\n"));
  }

 /*
  * open parameter file
  */
  param_file = open_param_file(ConfigFile);


 /*
  * get chipset definition
  */
  sscanf(findlabel(param_file, "ChipSet", LABEL_REQUIRED+LABEL_FROMSTART), "%*s %s", tempstr);
  chipset = findoption(tempstr, str_chipsets, NUM_CHIPSETS, "chip set");


 /*
  * Option parsing. Not all chips support all options
  */

  optionmask = parse_opt(param_file, chipset, str_chipsets[chipset]);

 /*
  * parse clock configuration in config file, fill structure with info
  */

  parse_clock(param_file, chipset, &ck);
  if (ck.type & CK_CLOCKCHIP)
  {
    PWARNING(("\n\tThe config file defines a ClockChip on this system.\n"\
              "\tA ClockChip is supposed to be able to create ANY clock.\n"\
              "\tClock probing is a bit useless then.\n"));
  }
  
 /*
  * Close the config file. We don't need it anymore.
  */

  fclose(param_file);
  
 /*
  * finally do it... maybe
  */
  
  if (program_hardware)
  {
    get_VGA_io_perm(chipset);

    /* sync disks if requested. Is there any way to do this and be SURE _all_ data has been flushed? */
    if (optionmask & OPT_SYNC)
    {
      PDEBUG(("Syncing disks...\n"));
      sync(); sleep(2); /* same as in "reboot" command. Hopefully enough */
    }

    unlock(chipset);

    getmode(&m, TRUE); /* get the current pixel clock so we can restore it */
    
    PDEBUG(("pixel clock before tests = %1.2f\n", m.pclock));
    for (n=0; n<ck.num_clocks; n++)
    {
      if (ck.clocks[n]==CKVAL_PROGRAMMABLE)
        PMESSAGE(("pixel clock %d is programmable\n", n));
      else
      {
        do_clock(chipset, &ck, ck.clocks[n], n, optionmask);
        getmode(&probe_m, TRUE);
        PMESSAGE(("pixel clock %d = %1.2f\n", n, probe_m.pclock));
      }
    }

   PDEBUG(("restoring old pixel clock\n"));
   do_clock(chipset, &ck, m.pclock, 0, optionmask);  /* restore old clock */
      
  }
  return(0);
}

