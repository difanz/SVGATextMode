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
 *** mode_msg: messages used for mode data grabbing
 ***/

#ifndef _MODE_MSG_H
#define _MODE_MSG_H


#define MSGSTR_GOLDEN_RATIO \
        "the `Golden Ratio' (X/Y is around 4/3) rule suggested this.\n"
        
msg_str mode_messages[] = {
      { MSG_VTIM_MOD ,
           "Vertical timings were modified because they wrapped around (probably at 1024)\n" },
      { MSG_HTIM_MOD ,
           "Horizontal timings were modified because they wrapped around (probably at 2048)\n" },
      { MSG_GOLDENRATIO_X_CGA ,
           "Horizontal timings were divided by 2 because\n"
           MSGSTR_GOLDEN_RATIO
           "This could be a CGA-emulation mode.\n" },
      { MSG_GOLDENRATIO_X_HICOLOR ,
           "Horizontal timings were divided by 2, 3 or 4 because\n"
           MSGSTR_GOLDEN_RATIO
           "This could be a Hi/TrueColor (15/16 or 24-bit) mode\n" },
      { MSG_GOLDENRATIO_Y_INTERLACE ,
           "Vertical Timings were multiplied by 2 because\n"
           MSGSTR_GOLDEN_RATIO
           "This could be an interlaced mode.\n" },
      { MSG_CLOCK_MEASUREMENTS ,
           "Only %d%% of measurements were considered 'valid',\n"
#ifdef DOS
           "If this were UNIX, then your system could just be under severe load.\n"
           "But... Since it isn't, MS-Winslows (DOS box) could be the reason.\n"
#else
           "Your system is probably under heavy load.\n"
#endif
           "The clock measurement you get can be inaccurate!\n" },
       { MSG_INTERLACE ,
           "Vertical timings were adjusted because\n"
           MSGSTR_GOLDEN_RATIO
           "This could be an interlaced mode.\n"
           "(if it is not, the adjustments are probably wrong)\n" },
       { MSG_BAD_HSYNC_STOP ,
           "Hsync stop is beyond total screen width.\n"
           "This mode was not constructed properly.\n" },
       { MSG_BAD_VSYNC_STOP ,
           "Vsync stop is beyond total screen height.\n"
           "This mode was not constructed properly.\n" },
       { MSG_PRESET_VTIM_MOD ,
           "Vertical timings were modified because the given screen height suggested this.\n" },
       { MSG_PRESET_HTIM_MOD ,
           "Horizontal timings were modified because the given screen width suggested this.\n" },
       { MSG_PRESET_X_WRONG ,
           "Error: The given screen width does not match with the data read from the VGA registers.\n" },
       { MSG_PRESET_Y_WRONG ,
           "Error: The given screen height does not match with the data read from the VGA registers.\n" },
       { MSG_16BPP ,
           "The given screen width suggests this is a 15/16BPP (HiColor) mode.\n" },
       { MSG_24BPP ,
           "The given screen width suggests this is a 24BPP (packed TrueColor) mode.\n" },
       { MSG_32BPP ,
           "The given screen width suggests this is a 32BPP (sparse TrueColor) mode.\n" },
       { MSG_IS_INTERLACE ,
           "The given screen height suggests this is an interlaced mode.\n" },
       { MSG_CGA ,
           "The given screen height suggests this is a CGA emulation mode.\n" },
       { MSG_WEIRD ,
           "The aspect ratio of this mode is very strange. The probed values are probably wrong somewhere.\n" },
       { MSG_Y_WRAP_COMPENSATE ,
           "The probed vertical size of this mode was extremely small.\n"
           "It has been increased with a multiple of 1024 until the timings looked more reasonable.\n" },
       { MSG_CLOCKPROBE_FAILED ,
           "The clock probe failed. This is most probably because there was no video clock at all.\n" },
       { -1 , NULL }
};

#endif

