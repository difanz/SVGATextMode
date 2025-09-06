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
 *** get/set VGAreg, a simple VGA register hacking program
 ***
 *** WARNING: since different SVGA cards use different extra address ranges in any 
 *** of the register sets, no checking is done to make sure you don't attempt to change
 *** a non-existing register!
 ***
 *** This is just a hacking tool! Use at your own risk. It was NOT intended to be 
 *** idiot proof! If you don't understand all this, then don't bother trying to use it.
 ***
 ***/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CHIPSETREC 1
#include "chipset.h"
#include "misc.h"
#include "vga_prg.h"
#include "unlock_svga.h"
#include "file_ops.h"
#include "string_ops.h"
#include "messages.h"
#include "cfg_structs.h"

char *CommandName;
#ifndef DOS
  char *ConfigFile=CONFIGFILE; 
#else
  char *ConfigFile="STMText.cfg";
#endif

extern int yyparse(void);
extern FILE *yyin;
#define param_file yyin

bool debug_messages=FALSE;

/* this adds all the global variables that hold the data from the config file. */
#include "cfg_data.h"

/* kludge. needed in VGA functions. I felt like chickening out this time. */
unsigned int reg_ioaddr=0x80; /* should be a safe default */


/*
 * Supported Register Sets
 */

#define REGSET_CRTC        0
#define REGSET_SEQ         1
#define REGSET_ATRCTL      2
#define REGSET_GRCTL       3
#define REGSET_MISC        4
#define REGSET_DAC_STATUS  5
#define REGSET_DAC_MASK    6
#define REGSET_DAC         7
#define REGSET_ET6000      8
#define REGSET_INDEX       9
#define REGSET_IO          10

#define RT_INDEX         1
#define RT_MEMMAP        2

t_str_token RegsetRec[] = {
  { "CRTC",       REGSET_CRTC       },
  { "SEQ",        REGSET_SEQ        },
  { "ATRCTL",     REGSET_ATRCTL     },
  { "GRCTL",      REGSET_GRCTL      },
  { "MISC",       REGSET_MISC       },
  { "DAC_STATUS", REGSET_DAC_STATUS },
  { "DAC_MASK",   REGSET_DAC_MASK   },
  { "DAC",        REGSET_DAC        },
  { "ET6000",     REGSET_ET6000     },
  { "idx",        REGSET_INDEX      },
  { "io",         REGSET_IO         },
  { "",           ENDREC            }
};

typedef const struct {
    int regset;
    int type;
} t_regset_type ;

t_regset_type RegsetType[] = {
  { REGSET_CRTC       , RT_INDEX  },
  { REGSET_SEQ        , RT_INDEX  },
  { REGSET_ATRCTL     , RT_INDEX  },
  { REGSET_GRCTL      , RT_INDEX  },
  { REGSET_MISC       , RT_MEMMAP },
  { REGSET_DAC_STATUS , RT_MEMMAP },
  { REGSET_DAC_MASK   , RT_MEMMAP },
  { REGSET_DAC        , RT_INDEX  },
  { REGSET_ET6000     , RT_INDEX  },
  { REGSET_INDEX      , RT_INDEX  },
  { REGSET_IO         , RT_MEMMAP },
  { -1                , ENDREC    }
};

#define USE_IO_ADDR ((RegsetType[regset].regset == REGSET_INDEX) \
                  || (RegsetType[regset].regset == REGSET_IO))

         
void outb_VGA_mem(int register_set, int data)
{
   switch(register_set)
  {
    case REGSET_MISC  : outb(data, VGA_MISC_W); break;
    case REGSET_DAC_STATUS : PWARNING(("DAC_STATUS is read-only.\n")); break;
    case REGSET_DAC_MASK : outb(data, DAC_MASK); break;
    case REGSET_IO : outb(data, reg_ioaddr); break;
    default: PERROR(("outb_VGA_mem: unknown register set %d\n",register_set));
  }
}

int inb_VGA_mem(int register_set)
{
   switch(register_set)
  {
    case REGSET_MISC  : return(inb(VGA_MISC_R)); break;
    case REGSET_DAC_STATUS  : return(inb(DAC_STATUS)); break;
    case REGSET_DAC_MASK : return(inb(DAC_MASK)); break;
    case REGSET_IO : return(inb(reg_ioaddr)); break;
    default: PERROR(("inb_VGA_mem: unknown register set %d\n",register_set));
  }
}

void outb_VGA_indexed(int register_set, int reg_index, int data)
{
   switch(register_set)
  {
    case REGSET_CRTC  : Outb_CRTC(reg_index,data); break;
    case REGSET_SEQ   : Outb_SEQ(reg_index,data); break;
    case REGSET_ATRCTL: Outb_ATR_CTL(reg_index,data); break;
    case REGSET_GRCTL : Outb_GR_CTL(reg_index,data); break;
    case REGSET_DAC   : OutRGB_DAC(reg_index,data); break; /* 24 bit! */
    case REGSET_ET6000: outb(data, PCIIOBase+reg_index); break;
    case REGSET_INDEX : outb(reg_index, reg_ioaddr); outb(data, reg_ioaddr+1); break;
    default: PERROR(("outb_VGA_indexed: unknown register set %d\n",register_set));
  }
}

int inb_VGA_indexed(int register_set, int reg_index)
{
   switch(register_set)
  {
    case REGSET_CRTC  : return(Inb_CRTC(reg_index)); break;
    case REGSET_SEQ   : return(Inb_SEQ(reg_index)); break;
    case REGSET_ATRCTL: return(inb_ATR_CTL(reg_index)); break;
    case REGSET_GRCTL : return(Inb_GR_CTL(reg_index)); break;
    case REGSET_DAC   : return(InRGB_DAC(reg_index)); break; /* 24 bit! */
    case REGSET_ET6000: return(inb(PCIIOBase+reg_index));
    case REGSET_INDEX : outb(reg_index, reg_ioaddr); return(inb(reg_ioaddr+1)); break;
    default: PERROR(("inb_VGA_indexed: unknown register set %d\n",register_set));
  }
}

char* int_to_bin(int num, int bits)
{
  static char binstr[sizeof(long int)+1];
  int i;
  
  for (i=0; i<bits; i++) binstr[i] = 0x30; /* char '0' */
  binstr[bits] = 0x00;
  
  for (i=0; i<bits; i++) binstr[bits-1-i] += ((num >> i) & 0x00000001);
  PDEBUG(("binstr = '%s'\n", binstr));
  return(binstr);
}

/* this is nearly identical to the one in cfglex.l. Consider merging them */

int local_find_token(t_str_token *rec, char *ch_str)
{
  int i = 0;
  while (rec[i].name!=CS_NONE)
  {
    if (!strcasecmp(rec[i].name_str,ch_str))
    {
      if (i != rec[i].name)
      {
        PERROR(("Internal error in setVGAreg.c: rec[].name/offset mismatch: i=%d, rec[i].name=%d\n",\
                 i, rec[i].name));
      }
      return i;
    }
    i++;
  }
  PERROR(("Unknown Register set `%s' on command line.\n"
          "  use the help (-h) option to get a list of all possible Register sets.\n", ch_str));
  return CS_NONE;
}


void usage(int setreg)
{
     int i=0;
     
     PMESSAGE(("version %s. (c) 1995-1998 Koen Gadeyne.\n"\
     "  Usage: %s [options] VGA_register_set [register_index] %s\n\n"\
     "  Options: -h  print usage information\n"\
     "           -n  Don't program VGA hardware\n"\
     "           -d  print debugging information\n"\
     "           -u  unlock chipset-specific registers\n"\
     "               (needs SVGAtextMode config file)\n"\
     "               You will need this to avoid a core dump on some cards.\n"\
     "           -p  produce 'pipeable' decimal output (= just numbers, no text)\n"\
     "           -x  produce 'pipeable' hex output\n"\
     "           -t <ConfigFile>\n"\
     "                 Use <ConfigFile> instead of the default (%s)\n"\
     "                 (only useful for '-u' option)\n"\
     "%s"\
     "\n"\
     "  register_index: An index in the specified VGA_register_set,\n"\
     "                  In decimal (e.g. '24'), hex ('0x18') or octal ('030').\n"\
     "                  Only needed when it is an indexed (indirect) VGA register.\n"\
     "%s"\
     "\n",\
     VERSION,\
     CommandName,\
     (setreg) ? "data" : "",\
     ConfigFile,\
     (setreg) ? \
     "           -s  `data' is a bitmask for setting bits in the register\n"\
     "           -c  `data' is a bitmask for clearing bits in the register\n"\
     : "",\
     (setreg) ? \
     "\n"\
     "  data: the data to program into the specified register (dec|hex|oct).\n"\
     "        (or the bitmask for the `-s' (set) or `-c' (clear) options)\n"\
     : ""));
     
     printf("  VGA_register_set: any of the following:\n ");
     i=0;
     while (RegsetRec[i].name != ENDREC)
     {
       printf(" `%s'", RegsetRec[i].name_str);
       i++;
     }
     printf("\n\n");
}


/***********************************************************************************************************/
 
int main (int argc, char* argv[])
{
  bool program_hardware=TRUE;
  bool unlock_chipset=FALSE;
  bool pipehex=FALSE;
  bool pipe=FALSE;
  bool arg_is_setmask=FALSE;
  bool arg_is_clearmask=FALSE;
  int regset = -1;
  int c;
  int tmpbyte=0;
  int regnum=0;
  bool setreg = FALSE; /* if TRUE: "getVGAreg" function, if FALSE, "setVGAreg" function */
  char* commandfilename;
  int data=0;
  
 /*
  * See what action is required: read or write VGA register
  */
    
  CommandName = argv[0];
  commandfilename = strrchr(CommandName, '/');
  if (commandfilename) commandfilename++;
  else commandfilename = CommandName;
  setreg = (!strncasecmp(commandfilename,"set",3));
  
 
 /*
  * command-line argument parsing
  */

  while ((c = getopt (argc, argv, "ndhupxsct:")) != EOF)
    switch (c)
    {
      case 'n': program_hardware=FALSE;
                break;
      case 'd': debug_messages=TRUE;
                break;
      case 'h': usage(setreg);
                exit(0);
                break;
      case 'u': unlock_chipset = TRUE;
                break;
      case 'p': pipe = TRUE;
                break;
      case 'x': pipehex = TRUE;
                break;
      case 't': ConfigFile=safe_strdup(optarg);
                break;
      case 's': arg_is_setmask=TRUE;
                break;
      case 'c': arg_is_clearmask=TRUE;
                break;
      case '?': usage(setreg);
                PERROR(("Bad option '-%c'\n",(char)optopt));
                exit(-1);
                break;
      default: PERROR(("getopt returned unknown token '%c'.\n",c));
    }
    
  PVERSION;

  PDEBUG(("'%cetVGAreg' function selected through command name '%s'\n", (setreg) ? 's' : 'g', commandfilename));

  /* get register set from commandline */
  if (argc<optind+1) PERROR(("Missing register set on commandline\n"));

  PDEBUG(("arg=%s\n", argv[optind]));
  regset = local_find_token(RegsetRec, argv[optind]);

  /* get extra index for REGSET_IO and REGSET_INDEX */
  if (USE_IO_ADDR)
  {
    optind++;
    if (argc<optind+1)
      PERROR(("Missing IO address for '%s' register set\n", RegsetRec[regset].name_str));
    reg_ioaddr = getint(argv[optind], "IO address", 0, 0xFFFF);
    PDEBUG(("IO address = %d (0x%x).\n", reg_ioaddr, reg_ioaddr));
  }

  /* get register index, if an indexed register set was specified */
  if (RegsetType[regset].type == RT_INDEX)
  {
    optind++;
    if (argc<optind+1)
      PERROR(("Missing register index for '%s' register set\n", RegsetRec[regset].name_str));
    regnum = getint(argv[optind], "register number", 0, 255);
    PDEBUG(("register index = %d (0x%x).\n", regnum, regnum));
  }

  /* get register data, if 'set'VGAreg command */
  if (setreg)
  {
    optind++;
    if (argc<optind+1) PERROR(("Missing register data\n"));
    data = getint(argv[optind], "register data", 0, regset==REGSET_DAC ? (256 << 16) -1 : 255);
    PDEBUG(("register data to write = %d (0x%x).\n", data, data));
  }
  optind++;  
  if (argc>optind) PWARNING(("Extra parameters (starting with '%s') ignored\n", argv[optind])); 
  

 /*
  * open parameter file if needed for unlocking chipset, use only chipset definition (until now)
  * This is a bit of an overkill, to parse the whole file just to get the chipset from it, but what the heck.
  */
  chipset = CS_VGA; /* if not defined: chipset = standard VGA */
  if (unlock_chipset)
  {
    param_file = open_param_file(ConfigFile);
    PDEBUG(("Parsing Config file...\n"));
    while (!feof(yyin)) { yyparse(); }
    fclose(param_file);
    if (chipset<0) PERROR(("No chipset defined in config file\n"));
  }  

/*
 * start doing something useful
 */
 
 if (program_hardware)
  {
     if (USE_IO_ADDR)
       get_IO_range(reg_ioaddr,2);
     get_VGA_io_perm(chipset);
     if (unlock_chipset) unlock(chipset); 
     if (RegsetType[regset].type==RT_INDEX)
     {
       if (arg_is_clearmask || arg_is_setmask)
       {
         tmpbyte = inb_VGA_indexed(regset, regnum);
         if (arg_is_clearmask)
           data = tmpbyte & ~data;
         if (arg_is_setmask)
           data = tmpbyte | data;
       }
       if (setreg) outb_VGA_indexed(regset, regnum, data);
       tmpbyte = inb_VGA_indexed(regset, regnum);
       if (pipe) printf("%d\n",tmpbyte);
       else if (pipehex) printf("0x%02x\n",tmpbyte);
       else printf("VGA '%s' register, index %d (=0x%x) contains %d (=0x%02x =b%s)\n",
                    RegsetRec[regset].name_str, regnum, regnum, tmpbyte, tmpbyte,
                    regset==REGSET_DAC ? int_to_bin(tmpbyte,24): int_to_bin(tmpbyte,8));
     }
     else
     {
       if (arg_is_clearmask || arg_is_setmask)
       {
         tmpbyte = inb_VGA_mem(regset);
         if (arg_is_clearmask)
           data = tmpbyte & ~data;
         if (arg_is_setmask)
           data = tmpbyte | data;
       }
       if (setreg) outb_VGA_mem(regset, data);
       tmpbyte = inb_VGA_mem(regset);
       if (pipe) printf("%d\n",tmpbyte);
       else if (pipehex) printf("0x%02x\n",tmpbyte);
       else printf("VGA '%s' register contains %d (=0x%02x =b%s)\n",
                    RegsetRec[regset].name_str, tmpbyte, tmpbyte,
                    regset==REGSET_DAC ? int_to_bin(tmpbyte,24): int_to_bin(tmpbyte,8));
     }
  }
  return(0);
}
