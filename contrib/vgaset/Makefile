# Makefile for vgaset. Adapted from the original Makefile.bsd for Linux by Koen Gadeyne

# Set HAS_USL_VTS if you are using SRV4 or (reportedly) Linux
HAS_USL_VTS=-DHAS_USL_VTS

# set COMMON to the pathname of the x386/common/compile.h header file
CC=gcc
CFLAGS= -O3 -s $(HAS_USL_VTS)

vgaset: vgaset.c svga.h
	$(CC) $(CFLAGS) vgaset.c -o vgaset

clean:
	rm -f vgaset *~ *.bak *.o
	