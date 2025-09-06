s/$(shell arch)/harhar/
s/default: user/default: dos/
s/all: user hack/all: dos doshack/
s/.depend/_depend/
s/DOS_CC/CC/
s/gcc-dos/gcc -I./
s/y.tab.o/y_tab.o/
s/y.tab.c/y_tab.c/
s/y.tab.h/y_tab.h/
s/cfglex.l/cfglex.dos/
s| DOS/| |
s/-MM/-MM -I. -DDOS/
s/$(MAKE)/make/
s/SVGATextMode/svgatext/
s|-C XFREE|-C XFREE -f makefile.dos -k |
