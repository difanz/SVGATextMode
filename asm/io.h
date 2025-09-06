#ifndef _ASM_IO_H
#define _ASM_IO_H

/*
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl.
 *
 * Adapted from the XFree86 common/compiler.h file (opposite argument order!)
 *
 * Only the GAS code is included here -- refer to the original file for other
 * assemblers.
 */

static __inline__ void
outb(val, port)
short port;
char val;
{
   __asm__ __volatile__("outb %0,%1" : :"a" (val), "d" (port));
}


static __inline__ void
outw(val, port)
short port;
short val;
{
   __asm__ __volatile__("outw %0,%1" : :"a" (val), "d" (port));
}

static __inline__ void
outl(val, port)
short port;
int val;
{
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(port)
short port;
{
   unsigned char ret;
   __asm__ __volatile__("inb %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inw(port)
short port;
{
   unsigned short ret;
   __asm__ __volatile__("inw %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inl(port)
short port;
{
   unsigned int ret;
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

#define outb_p(v,p) outb(p,v);inb(0x80)
#define inb_p(p) inb(p),inb(0x80)

#endif
