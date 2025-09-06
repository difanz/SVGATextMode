#
# Makefile for SVGA textmode manipulation program, XFREE stuff
#
# Author : Koen Gadeyne
#

# compiled without -Wall: too many warnings...

#CFLAGS are imported from top level

XFREEINC = -I./common -I./os-support -I./accel/s3 -I./include -I..
XFREECOMPAT = -include xfree_compat.h

XFREELIBS = libxf86_hw.a

OBJECTS = common_hw/I2061Acal.o common_hw/I2061Aset.o common_hw/I2061Aalt.o \
          common_hw/SC11412.o common_hw/S3gendac.o common_hw/Ti3025clk.o \
          common_hw/ICS2595.o common_hw/CirrusClk.o common_hw/Ch8391clk.o \
          common_hw/IBMRGB.o common_hw/STG1703clk.o common_hw/Ti3026clk.o \
          common_hw/ATTDac.o common_hw/TGUIclk.o \
          common_hw/xf86_PCI.o vgaPCI.o \
          mga_clock.o \
          mach64/mach64_mem_access.o mach64/mach64.o \
          riva128_clock.o \
          xfree_compat.o

#          mach64/mach64clockchips.o \

ASMOBJS = common_hw/IODelay.o


all: $(XFREELIBS) $(OBJECTS) $(ASMOBJS)


libxf86_hw.a: $(OBJECTS) $(ASMOBJS)
	$(AR) rcs libxf86_hw.a $(OBJECTS) $(ASMOBJS)

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(XFREEINC) $(XFREECOMPAT) $< -o $@


$(ASMOBJS): %.o: %.S
	$(CC) -c $(CFLAGS) $(XFREEINC) $< -o $@

clean:
	rm -f *~ *.o common_hw/*.o mach64/*.o *.bak $(XFREELIBS)
	rm -f DEADJOE 
	rm -f `find . -name '*~'`
	
mrproper: clean
	rm -f $(XFREELIBS)

depend: .depend

.depend::
	gcc -MM $(XFREEINC) $(XFREECOMPAT) *.c common_hw/*.c >.depend
	gcc -MM $(XFREEINC) common_hw/*.S >>.depend

#
# include .depend if it exists
#

include .depend

