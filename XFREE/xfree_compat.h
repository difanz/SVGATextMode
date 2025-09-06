/***
 *** xfree_compat.h: misc. stuff to get the XFREE code to compile. This is junk :-(
 *** Written by Koen Gadeyne
 ***
 ***/
 
#ifndef _COMPAT_H
#define _COMPAT_H

#define NeedFunctionPrototypes 1

extern int debug_level;
#define xf86Verbose (debug_level+1)

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef _XF86_HWLIB_H

/* ATTDac.c */
extern void xf86dactopel(
#if NeedFunctionPrototypes
        void
#endif
);

extern unsigned char xf86dactocomm(
#if NeedFunctionPrototypes
        void
#endif
);

extern unsigned char xf86getdaccomm(
#if NeedFunctionPrototypes
        void
#endif
);

extern void xf86setdaccomm(
#if NeedFunctionPrototypes
        unsigned char
#endif
);

extern void xf86setdaccommbit(
#if NeedFunctionPrototypes
        unsigned char
#endif
);

extern void xf86clrdaccommbit(
#if NeedFunctionPrototypes
        unsigned char
#endif
);

extern int xf86testdacindexed(
#if NeedFunctionPrototypes
        void
#endif
);

#endif


typedef int Bool;

#define ErrorF printf

typedef void *ScrnInfoRec;  /* dummy, not used by SVGATextMode */

extern Bool vga_open;


extern void GlennsIODelay();

extern int CirrusSetClock(int freq, Bool islaguna);

Bool xf86DisableInterrupts();

void xf86EnableInterrupts();

void s3OutTiIndReg(unsigned char reg, unsigned char mask, unsigned char data);
unsigned char s3InTiIndReg(unsigned char reg);

void s3OutTi3026IndReg(unsigned char reg, unsigned char mask, unsigned char data);
unsigned char s3InTi3026IndReg(unsigned char reg);


void s3ProgramTi3025Clock();

void set_ti_SOG(int chipset, Bool SOG);


extern void TGUISetClock(
#if NeedFunctionPrototypes
        int
#endif
);

#ifndef ulong
#define ulong unsigned long
#endif

Bool mga_get_pci_info();
void MGATi3026SetPCLK( long f_out, int bpp );
void MGATi3026SetMCLK( long f_out );
void midSetPixClock( ulong freq, int isG2 );
void MGAoutTi3026(unsigned char reg, unsigned char val);
        
Bool RIVA128ClockSelect( int clockspeed );

#endif

