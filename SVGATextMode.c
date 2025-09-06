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
 *** SVGATextMode.c: the great big wolf
 ***
 *** Edited on 132x64 screen. Hence the LONG lines. For best viewing conditions, use this program to make itself more readable :-)
 ***/


#include "misc.h"
#include "ttyresize.h"       /* must be first ! */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#ifndef DOS
#  include <linux/fs.h>
#  include <sys/ioctl.h>
#  include <sys/kd.h>
#  include <ctype.h>
#endif

/*
 * this is not very clean, but it throws out all unix tty resizing stuff, which also makes it compile clean in DOS
 * However, DOS DOES support resizing. Don't let that confuse you. This is a hack for easy compiling.
 */
#ifdef DOS
#  define NO_RESIZE
#endif


#define CHIPSETREC 1  /* included chipset definition struct */
#include "chipset.h"
#include "vga_prg.h" 
#include "unlock_svga.h" 
#include "special_svga.h" 
#include "setclock.h"
#include "validate.h"
#include "file_ops.h"
#include "messages.h"
#include "kversion.h"
#include "run_extprog.h"
#include "dump_cfgdata.h"
#include "XFREE/xfree_compat.h"

/*
 * this adds all the global variables that hold the data from the config file.
 */
#include "cfg_data.h"

/*
 * other global variables
 */

char *CommandName;
#ifndef DOS
  char *ConfigFile=CONFIGFILE; 
  #define DIR_SEP '/'
  #define DIR_SEP_S "/"
#else
  char *ConfigFile="STMText.cfg";
  #define DIR_SEP '\\'
  #define DIR_SEP_S "\\"
#endif
bool debug_messages=FALSE;
int debug_level=0;

#define LAST_CHAR(s) (((char *)s)[strlen(s) - 1])

/*
 * yacc parser functions
 */

extern int yyparse(void);
extern FILE *yyin;
#define param_file yyin

void usage()
{
     PMESSAGE(("version %s. (c) 1995-1998 Koen Gadeyne.\n"
     "  Usage: %s [options] [textmodelabel]\n"
     "  Options: -n  Don't program VGA hardware\n"
     "               (parse config file and report mode parameters)\n"
     "           -d  print debugging information\n"
     "           -h  print usage information\n"
     "           -r  don't run ResetProg\n"
     "           -f  don't run FontProg\n"
     "           -c  don't change pixel clock\n"
     "           -v  don't validate H/V frequencies with limits in config file\n"
     "           -s  scan and print out all valid text modes from the config file\n"
     "           -a  always do a full resize, even if changing to same size screen\n"
     "           -k  resize the screen, but don't tell the kernel about it.\n"
     "               side effect: no SIGWINCH will be sent to all programs\n"
     "               running on the console. The ResetProg is still called\n"
     "           -x  run even when not in textmode (can screw up your console!)\n"
     "           -m  allow 1x1 screen to avoid `VT_RESIZE: out of memory' error\n"
     "               (relatively dangerous - read SVGATextMode(8) manual first!)\n"
     "           -t <ConfigFile>\n"
     "               Use <ConfigFile> instead of the default (%s)\n"
     "           -o  force all standard VGA registers to a known state. See docs!\n"
     "               (useful to restore textmode when some graphics-program crashed)\n"
     "  Textmodelabel: an existing label from the config file\n"
     "    (optional when `DefaultMode' defined in config file)\n",
     VERSION, CommandName, ConfigFile));
}

/****************************************************************************************************************************/


int main (int argc, char* argv[])
{
  char *req_label=NULL;

  bool program_hardware=TRUE;
  bool scanmodes=FALSE;
  bool validate=TRUE;
  bool alwaysresize=FALSE;
  bool run_from_anywhere=FALSE;
  bool resize1x1=FALSE;
  bool force_all_regs=FALSE;
  bool no_kernel_resize=FALSE;
  int c;


  bool sresize=TRUE;         /* will be set when screen has been resized and vice-versa */

  bool run_resetprog=TRUE, run_fontprog=TRUE, run_pixclock=TRUE;

  t_mode *curr_textmode;

#ifndef DOS
  char *cptr; /* bc3c */

  const char *cdev = ConsoleDevice("");
  size_t clen = strlen(cdev);
#endif

 /*
  * command-line argument parsing
  */

  CommandName = argv[0];

  while ((c = getopt (argc, argv, "ndhrfcsakxmvot:")) != EOF)
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
      case 'r': run_resetprog=FALSE;
                break;
      case 'f': run_fontprog=FALSE;
                break;
      case 'c': run_pixclock=FALSE;
                break;
      case 's': scanmodes=TRUE;
                break;
      case 'm': resize1x1=TRUE;
                break;
      case 'v': validate=FALSE;
                break;
      case 'a': alwaysresize=TRUE;
                break;
      case 'k': no_kernel_resize=TRUE;
                break;
      case 'x': run_from_anywhere=TRUE;
                break;
      case 't': ConfigFile=safe_strdup(optarg);
                break;
      case 'o': force_all_regs=TRUE;
                break;
      case '?': usage();
                PERROR(("Bad option '-%c'\n",(char)optopt));
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }

  PVERSION;
  PDEBUG(("Debug level: %d\n", debug_level));
    

 /*
  * bc3c: don't allow running stm in some cases (user protection :-)
  */
#ifndef DOS
  if (!run_from_anywhere)
  {  
    if ((cptr=ttyname(STDIN_FILENO))) {
      PDEBUG(("running from %s\n", cptr));
      if (strcmp(cptr, "/dev/console"))
        if ( !((!strncmp(cptr, cdev, clen)) && (isdigit(cptr[clen]))) )
      {
        PERROR(("You can only run SVGATextMode from a virtual terminal "
                "(=not in X or remotely), unless you use the '-x' option.\n"));
      }
    } else {
      PERROR(("stdin is not a tty\n"));
    }
  }
#endif

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
   * switch between "scan" mode or normal mode
   */
  
  if (scanmodes)
  {
    scan_valid_modes(validate);
    exit(0);
  }
  else   /* normal program-a-new-textmode mode */
  {
   /*
    * look for a text mode label. If none, look for a "defaultmode" in the config file
    */
    req_label=NULL;

    /* if requested mode is on command line: use that one */
    if (optind < argc) req_label = argv[optind];
    
    /* defaultmode in config file is default when not overridden by command line */
    else if (defaultmode)
      {
        req_label = defaultmode;
        PDEBUG(("No mode label on command line: using '%s' as default mode\n", req_label));
      }

    /* none of the above: eject! eject! */
    else
      {
        usage();
        PERROR(("No textmode label on commandline, and no 'DefaultMode' in config file.\n"));
      }
  }


/* find last occurence of requested text mode line: This will allow
 * user-patched mode lines at end of TextConfig file to get preference over the
 * ones above it, which normally are the default ones (as suggested by Kenneth
 * Albanowski). Due to the way the file is parsed, the last lines are first in
 * the config file data structure, so we match the first occurence in the data base.
 */

  curr_textmode = text_mode_list;
  while (curr_textmode)
  {
    if (!strcasecmp(curr_textmode->name, req_label)) break;
    curr_textmode = curr_textmode->next;
  }
  
  if (!curr_textmode) PERROR(("Text Mode `%s' could not be found in the config file.\n", req_label));

  /* requested mode found in data base. show some information */
  PDEBUG(("Found requested text mode in config file:\n"));
  dump_mode(curr_textmode);
  
  /* scale the cursor to fit the chosen font size */
  cursor_start = (cursor_start * curr_textmode->FontHeight) / 32;
  cursor_end = (cursor_end * curr_textmode->FontHeight) / 32;
  PDEBUG(("Cursor start-end = %d-%d\n", cursor_start, cursor_end));

  /* scale underline position to fit current font size */
  if (underline_pos>=0) underline_pos = (underline_pos * curr_textmode->FontHeight) / 32;
  PDEBUG(("Underline pos. will be: %d\n", underline_pos));
 

 /*
  * show the user what mode will be programmed, and also check if it is allowed.
  */
  
  check_and_show_mode(curr_textmode, validate);

 /*
  * start changing some things now.
  */

  if (program_hardware)
  {
     sresize = check_if_resize(curr_textmode->cols, curr_textmode->rows);

     /* FIRST check for IO permissions, to avoid first resizing the screen, and then seeing that we can't
        write to the VGA regs */
         
     get_VGA_io_perm(chipset);
     
     /* sync disks if requested. Is there any way to do this and be SURE _all_ data has been flushed? */
     if (OFLG_ISSET(OPT_SYNC))
     {
       PMESSAGE(("Syncing disks...\n"));
       sync(); sleep(2); /* same as in "reboot" command. Hopefully enough */
     }
     
#ifndef DOS
#  ifndef NO_RESIZE
   
     if ((sresize || alwaysresize) && (!no_kernel_resize))
     {
      /*
       * first see if current kernel version supports resizing.
       */
       
       if (!check_kernel_version(1,1,54, "Virtual Terminal resizing"))
         PERROR(("Screen resizing not allowed (kernel version must be >= 1.1.54). Use a non-resizing text mode, or upgrade your kernel.\n"));

      /*
       * Resize the screen. Still needs LOTS more error checking to avoid dropping out in the middle, leaving
       * the user with a garbled screen.
       *
       * sresize will be TRUE when resizing tty's should be forced (due to the 2nd attempt do_VT_RESIZE will do
       * when not enough memory is free).
       *
       */

        /*
         * ALWAYS do a VT_RESIZE, even if we already did a VT_RESIZEX on a 1.3.3 or higher kernel, 
         * until those kernel programmers make this unambiguous
         */
      
       if (do_VT_RESIZE(curr_textmode->cols, curr_textmode->rows, resize1x1)) sresize=TRUE;
         
       if (check_kernel_version(1,3,3, "VT_RESIZEX"))
         {
           /*
            * VDisplay must de divided by 2 for DoubleScan modes,
            * or VT_RESIZEX will fail -- until someone fixes the kernel
            * so it understands about doublescan modes.
            */
           if (do_VT_RESIZEX(curr_textmode->cols,
                             curr_textmode->rows,
                             curr_textmode->VDisplay / (MOFLG_ISSET(curr_textmode, ATTR_DOUBLESCAN) ? 2 : 1),
                             curr_textmode->FontHeight,
                             curr_textmode->HDisplay/8*curr_textmode->FontWidth,
                             curr_textmode->FontWidth, resize1x1)) sresize=TRUE;
         }
         
      /*
       * resize terminals. If specified in "terminals" line, do just those.
       * If not specified, find out which ones are active, and resize those.
       * This is obsoleted by kernel v1.3.3 and up (they do it themselves upon
       * a call to VT_RESIZE)
       */
          
       if (!check_kernel_version(1,3,3, "Automatic TTY resizing (SIGWINCH)"))
       {
         if (p_terminals) resize_specified_vts(curr_textmode->cols, curr_textmode->rows);
           else resize_active_vts(curr_textmode->cols, curr_textmode->rows);
       }
     }

#  else 
     /*
      * no resizing support: do NOT allow resizing. Check if some stubborn moosehead tries anyway.
      */
      
      if (sresize) PERROR(("Resizing is not allowed (NO_RESIZE defined during compilation).\n"));
#  endif
#else   /* DOS */
      if (sresize) resize_DOS(curr_textmode->cols, curr_textmode->rows, curr_textmode->FontHeight);
#endif   

   /*
    * now get to the REAL hardware stuff !
    */

    Set_Textmode(force_all_regs);  /* just in case some jerk set us in graphics mode */

    unlock(chipset);

    special(chipset); /* change chipset-specific things, if needed */

    if (run_pixclock)
      do_clock(chipset, curr_textmode->pixelClock, curr_textmode->FontWidth);
    else PDEBUG(("Clock will NOT be programmed! (due to command line switch '-c') !\n"));
    
    Set_MAX_SCANLINE (curr_textmode->FontHeight);
    
    set_V_timings(curr_textmode->VDisplay, curr_textmode->VSyncStart, curr_textmode->VSyncEnd, curr_textmode->VTotal);
    set_H_timings(curr_textmode->HDisplay, curr_textmode->HSyncStart, curr_textmode->HSyncEnd, curr_textmode->HTotal);
    
    interlace(chipset, curr_textmode);
    
    Outbit_CRTC(0x09, 7, MOFLG_ISSET(curr_textmode, ATTR_DOUBLESCAN) ? 1 : 0);

    Set_CURSOR_START(cursor_start) ; Set_CURSOR_END(cursor_end);
    
    if (underline_pos>=0) Set_Underline_location(underline_pos);

    Set_HSYNC_POLARITY(curr_textmode->hpol) ; Set_VSYNC_POLARITY(curr_textmode->vpol);

    Set_HORIZ_SHIFT(curr_textmode->hshift);

    /* set sync-on-green on special RAMDAC's */
    if ( (clock_data.clockchiptype==CLKCHIP_TI3026) || (clock_data.clockchiptype==CLKCHIP_TI3025) )
      set_ti_SOG(chipset, (OFLG_ISSET(OPT_SOG)) );
      
    Outbit_ATR_CTL(0x10, 3, OFLG_ISSET(OPT_16COLOR) ? 0 : 1);

#ifndef DOS
    /* set console to text mode */
    ioctl(opentty("/dev/console"), KDSETMODE, KD_TEXT);
#endif

    if (set_charwidth(curr_textmode->FontWidth)) PERROR(("Illegal character width: %d\n",curr_textmode->FontWidth));

    Outb_ATR_CTL(0x11, bordercolor);
    
    SCREEN_ON;
    
    /* could do a safe_wait_vsync() here to check if we have some refresh */

  }
  
 /*
  * call the external font loading program, if enabled by the 'option loadfont' line
  */

 if ((program_hardware) && (chipset==CS_S3))
 {
   /* S3 high speed text mode cannot be run when font loading is not enabled */
   S3_StartHSText_FontLoad(curr_textmode->pixelClock, FALSE);
 }
 
 if (OFLG_ISSET(OPT_LOADFONT) && (program_hardware) && (run_fontprog))
 {
   if (!(font_data.font_table[curr_textmode->FontWidth-8][curr_textmode->FontHeight-1]))
   {
     PWARNING(("Font loading enabled, but no font specified for %dx%d text mode.\n", curr_textmode->FontWidth, curr_textmode->FontHeight));
     PWARNING(("No font will be loaded.\n"));
   }
   else
   {
     char tempstr[1024]="";
     int result=0;
     snprintf(tempstr, sizeof(tempstr), "%s ", font_data.fontprogpath);
     if (font_data.fontpath)
     {
       strcat(tempstr, font_data.fontpath);
       if (LAST_CHAR(tempstr) != DIR_SEP)
         strcat(tempstr, DIR_SEP_S);
     }
     strcat(tempstr, font_data.font_table[curr_textmode->FontWidth-8][curr_textmode->FontHeight-1]);

     if (chipset==CS_S3)
     {
       PDEBUG(("Executing external command %s'\n", tempstr));
       S3_StartHSText_FontLoad(curr_textmode->pixelClock, TRUE);
       result = Run_extern_Prog_pipe(tempstr);
       S3_EndHSText_FontLoad();
       show_extout();
       if (result<0) PWARNING(("External program call `%s' through pipe failed.\n", tempstr));
     }
     else
     {
       Run_extern_Prog(tempstr);
     }
   }
 }

// Hacking the hack; ResetProg might be useful in DOS, too. - Wolfram, Jan 06
#ifdef DOS
#  undef NO_RESIZE
#endif

#ifndef NO_RESIZE
 /*~
  * call the reset program (which could be used to notify applications about the screen changes)
  * But only when the screen has been resized.
  *
  * Of course, nobody needs this when resizing is impossible (NO_RESIZE defined)
  */

 if ((resetprogpath) && (program_hardware) && (run_resetprog))
 {
    char tempstr[1024]="";
    if (!sresize && !alwaysresize)
      PDEBUG(("Screen not resized. ResetProg not executed.\n"));
    else
    {
      snprintf(tempstr, sizeof(tempstr), "%s %d %d", resetprogpath, curr_textmode->cols, curr_textmode->rows);
      Run_extern_Prog(tempstr);
    }
  }       
#endif

#ifdef DOS
#  define NO_RESIZE
#endif
  return(0);
}

