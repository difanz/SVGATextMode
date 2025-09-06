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
 *** Linux tty resizing code for SVGATextMode.
 ***
 *** DOS resizing code for SVGATextMode.
 ***    (c) 1995 Stephen Lee
 ***/

#define USE_MMAP  0   /* 1: use mmap() ; 0: use malloc() */

#ifndef DOS

#include <linux/version.h>      /* the <linux/config.h> file states we should NOT use <linux/version.h>... */
#if LINUX_VERSION_CODE < 66382
#  define __KERNEL__
#  include <linux/tty.h>	/* for MAX_NR_CONSOLES  -- __KERNEL__ is defined to get access to this parameter */
#  undef __KERNEL__
#else
#  include <linux/tty.h>	/* for MAX_NR_CONSOLES */
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <asm/page.h>	/* for PAGE_SIZE */
#include <linux/vt.h>   /* for VT_RESIZE */
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <errno.h>      /* for libc6 and Alpha */

#include "misc.h"
#include "messages.h"
#include "cfg_structs.h"
#include "file_ops.h"
#include "ttyresize.h"
#include "kversion.h"

#if USE_MMAP
#  include <sys/types.h>
#  include <linux/fs.h>
#  include <sys/mman.h>
#endif

void get_ttysize(int fd, const char *devname, struct winsize *this_winsize)
{
  if (ioctl(fd, TIOCGWINSZ, this_winsize))
  {
     close(fd);
     perror("TIOCGWINSZ");
     PERROR(("Could not read Terminal size for %s\n", devname));
  } 
}

#ifndef NO_RESIZE

void set_ttysize(int fd, const char *devname, struct winsize *this_winsize, int cols, int rows)
{
  /* no need to skip this if the screen is not resized. The kernel already does that */

  PDEBUG(("set_tysize: Resizing %s from %dx%d to %dx%d\n", devname, this_winsize->ws_col, this_winsize->ws_row, cols, rows));
  this_winsize->ws_col = cols;
  this_winsize->ws_row = rows;
  if (ioctl(fd, TIOCSWINSZ, this_winsize))
  {
     close(fd);
     perror("TIOCSWINSZ");
     PERROR(("Could not set Terminal size for %s\n", devname));
  }
}

void resizetty(const char *devicename, int cols, int rows)
{
  struct winsize my_winsize;
  int fd;

  fd = opentty(devicename);
  get_ttysize(fd, devicename, &my_winsize);
  set_ttysize(fd, devicename, &my_winsize, cols, rows);
  close(fd);
}

/*
 * Make real RAM free, by allocating and then releasing it.
 */

int make_ram_free(size_t bytes)
{
  /*
   * 1. Linux has a "lazy" malloc().  Memory is not actually allocated
   * until it is used.  Conclusion: We must use the RAM to actually
   * get it.
   *
   * 2. Linux has a "lazy" free().  Memory is not (necessarily) actually
   * freed until the program exits.  Conclusion: We must fork off a
   * child so that it can exit.
   *
   * Written by Michael Shields <shields@tembel.org>.
   */

#if USE_MMAP
  caddr_t mm;
#else
  pid_t pid;
  int status;
  char *p;
  size_t i;
#endif

  if (!bytes)
    return 0;
    
  PDEBUG(("Freeing %d bytes of RAM for VT_RESIZE (using %s())\n", bytes, USE_MMAP==1 ? "mmap" : "malloc"));

#if USE_MMAP
  mm = mmap(0, bytes, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
  if ( (int)mm == -1 )
  {
    PDEBUG(("Not enough memory for VT_RESIZE. Attempting to free some disk buffers...\n"));
    shrink_buffers(0);
    mm = mmap(0, bytes, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
    if ( (int)mm == -1 )
    {
      perror("mmap()");
      return(-1);
    }
  }
  munmap(mm, bytes); 
  return(0);
#else
  pid = fork();

  switch(pid) {
  case -1:
    return -1;

  case 0:
    p = (char *) malloc(bytes);
    if (!p)
      exit(1);

    /* Dirty the pages.  This is better than calloc(). */
    for (i = 0; i < bytes; i += PAGE_SIZE)
        p[i] = 1;

    free(p);
    exit(0);

  default:
    waitpid(pid, &status, 0);
    return (WIFEXITED(status) && !WEXITSTATUS(status)) ? 0 : -1;
  }
#endif  
}


/*
 * VT_RESIZE lets the KERNEL know that the screen has been resized,
 * so it writes the chars in the correct place in video memory
 *
 * This is done BEFORE programming any hardware, so if it gives an "out of memory" error on heavily loaded machines,
 * If it exits with an "ioctl: invalid argument" (because the kernel doesn't support VT_RESIZE),
 * the screen isn't left all messed up. (suggested by Alessandro Rubini).
 *
 * NOTE: VT_RESIZE only sets the kernel parameters for correctly writing into the VGA memory. It does NOT resize
 * terminals (the stty stuff). So it only makes the HARDWARE behave correctly. Terminals are another story.
 *
 */


bool try_resize(int fd, void* p_struct_size, int memsize, int cmd)
{
  int cnt;

  /* The Real Thing (TM) ... */
  if (!ioctl(fd, cmd, p_struct_size)) return(TRUE);   /* everything went OK */

  /* do a few attempts to get enough memory for VT_RESIZE, each time more, hoping we'll get it in the end */
  for (cnt=1; cnt<=4; cnt++)
  {
    PDEBUG(("VT_RESIZE: Could not get memory. Trying to free some (%d), and attempting again...\n", memsize*cnt));
    if (make_ram_free(memsize*cnt))
      PERROR(("malloc(): Could not get %d bytes of memory.\n"
              "  Close some applications and try again, or add some swap space...\n", memsize*cnt));

    /* The Real Thing (TM) ... */
    if (!ioctl(fd, cmd, p_struct_size)) return(TRUE);   /* everything went OK */
  }
  return(FALSE);
}

bool generic_VT_RESIZE(void* p_struct_size, void* dummy, int allow1x1, int memsize, int cmd, char* descr)
{
  int fd;

  PDEBUG(("%s\n", descr));
  fd = opentty( ConsoleDevice("0") );

  if (try_resize(fd, p_struct_size, memsize, cmd)) return(FALSE);

  /* still no luck... if the error is not "out of memory", we don't know what to do and abort. */
  perror(descr); if (errno!=ENOMEM) PERROR(("VT_RESIZE returned error %d\n", errno));

  /* if we are not allowed to resize to a 1x1 screen, we have no more options but to abort */
  if (!allow1x1)
  {
    PERROR(("Not enough free _physical_ RAM to resize the screen.\n"
     "  SVGATextMode needs a fairly large block of _contiguous_ memory.\n"
     "  You might have enough free memory, but then it's fragmented too much.\n"
     "  Consider using the '-m' command line switch to clear more memory,\n"
     "  BUT this clears ALL screens in the process! (Read the manual first!)\n"));
  }

  /* Get more memory by temporarily resizing the screen to 1x1, and THEN back to the new size */
  PMESSAGE(("Not enough free RAM. Trying via 1x1 screen...\n"));
  if (ioctl(fd, cmd, dummy))
  {
      /* This is NOT good. We're in trouble */ 
      perror(descr);
      PERROR(("Could not even resize screen to 1x1 to free more RAM.\n"
       "  You REALLY need to free some memory.\n"));
  }
  
  /* now try again, but first reclaim the RAM. It could have been snatched away again. */
  if (try_resize(fd, p_struct_size, memsize, cmd))
  {
    PWARNING(("Could not get enough RAM for %s.\n"
     "  Tried via 1x1 screen to get even more memory.\n"
     "  All screens will be erased, except those that redraw themselves.\n", descr));
    return(TRUE);  /* "TRUE" means we went via a 1x1 screen to succesfully resize the screen */
  }

  /* At this point, we're in serious trouble... */
  PWARNING(("Could not set kernel screen size parameters.\n"
     "  Serious trouble! (you are probably left with a 1x1 screen...)\n"));

  return(TRUE);

}

int do_VT_RESIZE(int cols, int rows, int allow1x1)
{
  struct vt_sizes my_vt_size, dummy_vt_size;      /* passes the new screen size on to the kernel */

  /* We need two bytes for each character (character + attribute), per console. */
  int ram_needed = cols * rows * 2 * MAX_NR_CONSOLES;

  my_vt_size.v_rows = rows;
  my_vt_size.v_cols = cols;
  my_vt_size.v_scrollsize = 0; /* kernel tries to get as many scroll-back lines as possible by itself (?) */
  
  dummy_vt_size.v_rows = 1;
  dummy_vt_size.v_cols = 1;
  dummy_vt_size.v_scrollsize = 0;

  return(generic_VT_RESIZE(&my_vt_size, &dummy_vt_size, allow1x1, ram_needed, VT_RESIZE, "VT_RESIZE"));
}

/*
 * if VT_RESIZEX not supported (i.e. when compiling on < 1.3.3 kernels), define it.
 * this is just te keep the compiler happy
 */
 
#ifndef VT_RESIZEX
#  define VT_RESIZEX  0x560A
   typedef struct vt_consize { 
      ushort v_rows; ushort v_cols; ushort v_vlin; ushort v_clin; ushort v_vcol; ushort v_ccol;
    } vt_consize;
#endif


int do_VT_RESIZEX(int cols, int rows, int vlin, int clin, int vcol, int ccol, int allow1x1)
{
  struct vt_consize my_vt_size;      /* passes the new screen size on to the kernel */
  struct vt_consize dummy_vt_size = { 1 , 1 , 1 , 1 , 1 , 1 };
  int ram_needed = cols * rows * 2 * MAX_NR_CONSOLES;

  my_vt_size.v_rows = rows;
  my_vt_size.v_cols = cols;
  my_vt_size.v_vlin = vlin;
  my_vt_size.v_clin = clin;
  my_vt_size.v_vcol = vcol;
  my_vt_size.v_ccol = ccol;

  PDEBUG(("VT_RESIZEX(cols=%d,rows=%d,vlin=%d,clin=%d,vcol=%d,ccol=%d)\n",cols, rows, vlin, clin, vcol, ccol));
  
  return(generic_VT_RESIZE(&my_vt_size, &dummy_vt_size, allow1x1, ram_needed, VT_RESIZEX, "VT_RESIZEX"));
}

/*
 * resize all specified VT's.
 *
 * This function resizes the TERMINALS (equivalent to "stty rows ... cols ...) , if any were given in a "Terminals" line .
 * Another fine suggestion by Kenneth Albanowski
 *
 * It complements VT_RESIZE, which only resizes "the hardware". This resizes "the software".
 */
  
void resize_specified_vts(int cols, int rows)
{
  t_terminals *curr_term;

  PDEBUG(("Resizing all terminals specified in 'Terminals' line (when needed)\n"));
  
  curr_term = p_terminals;
  while (curr_term)
  {
    resizetty(curr_term->name, cols, rows);
    curr_term = curr_term->next;
  }
}


/*
 * Resize the VT's reported active from VT_GETSTATE, unfortunately
 * the return param is a short so we can only hope to know about the
 * first 16.  This won't do anything to VT's beyond those first 16.
 * Returns 0 on success and -1 on failure.
 *
 * If you have >16 VT's to resize, you'll have to use the "Terminals" line.
 *
 * Written by Reuben Sumner <rasumner@undergrad.math.uwaterloo.ca>
 * and adapted a little by kmg... (sorry, I couldn't resist)
 */

void resize_active_vts(int cols, int rows)
{
   int fd;
   struct vt_stat vst;
   unsigned short mask;
   int i;
   char devicenum[5];
   
   PDEBUG(("Resizing all active VT's when needed\n"));
   
   fd = opentty( ConsoleDevice("0") );
   if (ioctl(fd, VT_GETSTATE, &vst))
   {
      perror("VT_GETSTATE");
      PERROR(("Could not do VT_GETSTATE on %s\n", ConsoleDevice("0")));
   }
   close(fd);

  /* vst.v_state is a 16-bit "short": bit 0 = tty0, bit 1 = tty1 , ..., bit 15 = tty15
   * tty0 is a special case: it is the _current_ console (also /dev/tty0)
   * /dev/tty0 does not exist on some systems, plus it is also the current console (e.g. 
   * /dev/tty3), so we don't resize it, avoiding an error on some systems, and also avoiding
   * resizing the same tty twice.
   */
    
   for (mask = 2, i = 1; i < 16; i++, mask <<= 1)
   {
      if ((vst.v_state & mask) != 0)       /* only resize active VT's */
      {
        snprintf(devicenum, sizeof(devicenum), "%d", i);
        resizetty(ConsoleDevice(devicenum), cols, rows);
      }
   }
}

#endif /* NO_RESIZE */

int check_if_resize(int cols, int rows)
{
  struct winsize my_winsize;
  int fd;
  const char *devicename = ConsoleDevice("0");
  
  PDEBUG(("Checking if new mode requires screen resizing (from %s)\n", devicename));

  fd = opentty(devicename);
  get_ttysize(fd, devicename, &my_winsize);
  close(fd);
  return((my_winsize.ws_col != cols) || (my_winsize.ws_row != rows));
}


#else

/***
 *** DOS resizing code for SVGATextMode.
 *** (c) 1995 Stephen Lee
 *
 * April 97: Screen/font-restoring by <A.Borrmann@tu-bs.de>
 *
 ***/

/* for DJGPP v2 */
#include <pc.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <string.h>

#include "messages.h"

int resize_DOS(int cols, int rows, int fontsize)
{
  unsigned bioscolsp = 0x0044a;
  unsigned biosrowsp = 0x00484;
  unsigned short old_cols = _farpeekw(_dos_ds, bioscolsp);
  unsigned char  old_rows = _farpeekb(_dos_ds, biosrowsp);
  int x,y,x_min,y_min,y_start;
  
  unsigned short *copy;
  
  _go32_dpmi_registers   regs;
  
  copy=alloca(old_cols*(old_rows+1)*2);

  if (copy)
  {
    dosmemget(0xB8000,old_cols*(old_rows+1)*2,copy);

    x_min=(old_cols<cols)?old_cols:cols;
    y_min=((old_rows<rows)?old_rows:rows)+1;
    // If new size is smaller, just copy the last (=recent) part - Wolfram, Jan 06
    y_start=(old_rows>rows)?old_rows-rows:0;
    for (y=0;y<y_min;y++)
    {
      dosmemput(&(copy[old_cols*(y+y_start)]),2*x_min,0xB8000 + 2*cols*y);
      // do clreol:
      for (x=x_min;x<cols;x++) _farpokew(_dos_ds, 0xB8000 + 2*(cols*y + x), 0x0720);
    }
    for (y=y_min;y<rows;y++)
      for (x=0;x<cols;x++) _farpokew(_dos_ds, 0xB8000 + 2*(cols*y + x), 0x0720);
  }
    
  // The bios "resize" code:
  _farpokew(_dos_ds, bioscolsp, cols);
  _farpokeb(_dos_ds, biosrowsp, rows - 1);
	 
  // Load ROM-font, 16 pixel height as default
  memset(&regs,0,sizeof(regs));
  switch(fontsize)
  {
    case  8 : regs.x.ax=0x1102; break;
    case 14 : regs.x.ax=0x1101; break;
    case 16 :
    default : regs.x.ax=0x1104; break;
  }
  
  _go32_dpmi_simulate_int(0x10, &regs);

  _farpokeb(_dos_ds, 0x485, fontsize);

  // Check if cursor is out of bounds - Wolfram, Jan 06
  ScreenGetCursor(&y, &x);
  if (x > cols - 1)
    x = 0;
  if (y > rows - 1)
    y = rows - 1;
  ScreenSetCursor(y, x);

  return 0;
}

int check_if_resize(int cols, int rows)
{
  /* We will always resize, because some DOS-programs change VGA-registers
   * but not BIOS data. So, there is no reliable way to check this. 
   * - Wolfram, Jan 06 
   */
  return(TRUE);
}

#endif
