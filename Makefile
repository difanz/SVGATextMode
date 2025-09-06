#
# Makefile for SVGA textmode manipulation program
#
# Authors : Koen Gadeyne
#           Linux/Alpha port: David Mosberger-Tang
#           First DOS port of grabmode: Kenneth Albanowski
#           All other DOS porting: - Stephen Lee
#                                  - DJGPP 2.0
#
# for making DOS tools, use "make dos" instead of just "make" when compiling
# from DOS.
#
# WARNING: make sure you do a "make mrproper" before switching from DOS to
# Linux compilation or vice versa.


# ARCH=$(shell arch)
# The Debian package should always be built with i386 flags anyway
ARCH = $(shell dpkg-architecture -qDEB_BUILD_ARCH)

CFLAGS_alpha =
CFLAGS_i386  = -pipe -O2 -Wall -Wpointer-arith -Wnested-externs 
CFLAGS_i486  = $(CFLAGS_i386) -m486 -fomit-frame-pointer
CFLAGS_i586  = $(CFLAGS_i486) -fno-strength-reduce -malign-loops=2 -malign-jumps=2 -malign-functions=2
CFLAGS_i686  = $(CFLAGS_i586)
CFLAGS_amd64 = $(CFLAGS_i386)
LDFLAGS_alpha =
LDFLAGS_i386  =
LDFLAGS_i486  = $(LDFLAGS_i386)
LDFLAGS_i586  = $(LDFLAGS_i386)
LDFLAGS_i686  = $(LDFLAGS_i386)
LDFLAGS_amd64 = $(LDFLAGS_i386)

YACC     = bison -y
LEX      = flex -l
LEXFLAGS = -i
YFLAGS   =

CDEBUGFLAGS =   # -ggdb

DOS_CC = gcc-dos -s -DDOS
DOS_AR = ar-dos
WIN_CC = gcc -s -DDOS -DWIN

CPPFLAGS = $(CONF_FLAGS) $(DEF_CONF) $(DEF_CLOCKCONF) $(STM_VERSION)
CFLAGS = $(CDEBUGFLAGS) $(CFLAGS_$(ARCH)) -IXFREE/include
LDFLAGS = $(LDFLAGS_$(ARCH))

export CFLAGS

# added for Debian by Ron Lee
DESTDIR =

# installation path for SVGATextMode
#INSTBINDIR = /usr/sbin
INSTBINDIR = $(DESTDIR)/sbin

# installation path for TextConfig. This is also the path that will be
# compiled into SVGATextMode.
#INSTCONFDIR = /etc
INSTCONFDIR = $(DESTDIR)/etc

# installation path for the manual files
#INSTMANDIR = /usr/man
INSTMANDIR = $(DESTDIR)/usr/share/man

# misc configuration flags.
#
# possible flags are:
#
#     -DRUN_SECURE         Renounce superuser rights immediately after
#                          getting permission for VGA registers, so the
#                          external programs are NOT run as root.
#                          This is only useful if you set the SUID bits on.
#
#     -DNO_DEBUG           Don't include debugging code (for "-d" option)
#                          in all of the tools.
#
#     -DNO_RESIZE          Do not include screen resizing code. This will prohibit
#                          SVGATextMode from resizing the screen, leaving only screen
#                          enhancement functionality. Kernel versions
#                          prior to 1.1.54 NEED this to compile and run without errors.
#
#     -DDOSYNCRESET        Will do a synchronous reset of the timing sequencer before
#                          changing clocks. This seems to crash some ET4000's, although
#                          the data books recommend it...
#
#     -DDOS                Compile for DOS, using a DOS compiler. DJGPP 2.0 recommended.
#                          You MUST (!) use DJGPP 2.0 (released Sept 1995) for this!!!
#                          Currently all instances of "y.tab.*" must be replaced by
#                          "y_tab.*" by hand for compiling under DOS.
#
#     -DVGA_CAN_DO_64KB    This will allow text modes with up to 32k chars instead of the
#                          default 16k. You need a kernel with "VGA_CAN_DO_64KB" defined
#                          in /usr/src/linux/drivers/char/vga.c, so the kernel knows how
#                          to use 64kb of VGA memory instead of 32k.
#

CONF_FLAGS = 

############  END  OF  USER  CONFIGURATION  #################################


VERSION_NUM = 1.9


STM_VERSION = -DVERSION=\"$(VERSION_NUM)\"

#
# Some directories and files
#

SRCDIR = SVGATextMode-${VERSION_NUM}-src
BINDIR = SVGATextMode-${VERSION_NUM}-bin
DOSDIR = SVGATextMode-${VERSION_NUM}-dos

CONFIGFILE= $(INSTCONFDIR)/TextConfig
CLOCKCONFIGFILE= $(INSTCONFDIR)/ClockConfig
DEF_CONF = -DCONFIGFILE=\"$(CONFIGFILE)\"
DEF_CLOCKCONF = -DCLOCK_CONF_FILE=\"$(CLOCKCONFIGFILE)\"
XFREELIBS = XFREE/libxf86_hw.a
XFREELINK = -LXFREE -lxf86_hw

USERPROGS = SVGATextMode stm clockprobe grabmode
HACKPROGS = setVGAreg getVGAreg setpalette getpalette set80 ClockProg 

BINARIES = $(USERPROGS) $(HACKPROGS)

#
# standard targets
#

default: user

all: user hack

$(XFREELIBS)::
	$(MAKE) -C XFREE all


user: $(USERPROGS)

hack: $(HACKPROGS)

#
# Object files
#

STM_OBJECTS = ttyresize.o messages.o vga_prg.o setclock.o file_ops.o SVGATextMode.o \
              validate.o run_extprog.o wait_vsync.o clockchip.o std_clock.o cfglex.o \
              y.tab.o misc.o dump_cfgdata.o kversion.o unlock_svga.o special_svga.o \
              console_dev.o

STM_DOS_OBJECTS = ttyresize.o messages.o vga_prg.o setclock.o file_ops.o SVGATextMode.o \
              validate.o run_extprog.o wait_vsync.o clockchip.o std_clock.o cfglex.o \
              y.tab.o misc.o dump_cfgdata.o unlock_svga.o special_svga.o
              
CP_OBJECTS = messages.o vga_prg.o setclock.o file_ops.o string_ops.o ClockProg.o \
             validate.o run_extprog.o wait_vsync.o clockchip.o std_clock.o cfglex.o \
             y.tab.o misc.o dump_cfgdata.o unlock_svga.o special_svga.o

SET80_OBJECTS = set80.o vga_prg.o messages.o ttyresize.o file_ops.o kversion.o console_dev.o

SET80_DOS_OBJECTS = set80.o vga_prg.o messages.o ttyresize.o file_ops.o

VGAREG_OBJECTS = setVGAreg.o vga_prg.o file_ops.o string_ops.o messages.o \
                 cfglex.o y.tab.o misc.o unlock_svga.o

PAL_OBJECTS = setpalette.o vga_prg.o string_ops.o messages.o file_ops.o kversion.o console_dev.o

PAL_DOS_OBJECTS = setpalette.o vga_prg.o string_ops.o messages.o file_ops.o

GRAB_OBJECTS = grabmode.o modedata.o probe.o wait_vsync.o vga_prg.o messages.o \
               string_ops.o user_time.o

#PCLKS_OBJECTS = probeclocks.o messages.o vga_prg.o setclock.o file_ops.o \
#                run_extprog.o clockchip.o std_clock.o modedata.o probe.o wait_vsync.o
             

#
# UNIX targets
#

SVGATextMode: $(STM_OBJECTS) $(XFREELIBS)
	$(CC) $(LDFLAGS) $(STM_OBJECTS) $(XFREELINK) -o $@
	
stm: SVGATextMode
	@if [ ! -x SVGATextMode ]; then \
		make SVGATextMode; \
	fi
	ln -sf SVGATextMode stm

ClockProg: $(CP_OBJECTS) $(XFREELIBS)
	$(CC) $(LDFLAGS) $(CP_OBJECTS) $(XFREELINK) -o $@
	
set80: $(SET80_OBJECTS)
	$(CC) $(LDFLAGS) $(SET80_OBJECTS) -o $@
	
setVGAreg: $(VGAREG_OBJECTS)
	$(CC) $(LDFLAGS) $(VGAREG_OBJECTS) -o $@
	
getVGAreg: setVGAreg
	ln -sf setVGAreg getVGAreg	
	
setpalette: $(PAL_OBJECTS)
	$(CC) $(LDFLAGS) $(PAL_OBJECTS) -o $@
	
getpalette: setpalette
	ln -sf setpalette getpalette	
	
clockprobe: grabmode
	ln -sf grabmode clockprobe

grabmode: $(GRAB_OBJECTS)
	$(CC) $(LDFLAGS) $(GRAB_OBJECTS) -o $@

y.tab.c y.tab.h: cfgfile.y messages.c messages.h
	$(YACC) $(YFLAGS) -d $<

cfglex.c: cfglex.l y.tab.h messages.c messages.h
	$(LEX) $(LEXFLAGS) -t $< > $@


#
# DOS targets.
#
# DOS compilation needs different GCC, with some extra options
#

DOSPROGS = grabmode.exe stm.exe clkprobe.exe
DOSHACKPROGS = clkprog.exe setvgarg.exe getvgarg.exe set80.exe setpal.exe getpal.exe

alldos: dos doshack

DOS dos:
	make CC="$(DOS_CC)" AR="$(DOS_AR)" _dos
	
doshack:
	make CC="$(DOS_CC)" AR="$(DOS_AR)" _doshack

win:
	make CC="$(WIN_CC)" _dos

_dos: $(DOSPROGS)

_doshack: $(DOSHACKPROGS)

grabmode.exe: $(GRAB_OBJECTS)
	$(CC) $(CFLAGS) $(GRAB_OBJECTS) -o DOS/grabmode.exe

clkprobe.exe: grabmode.exe
	DOS/DOSln.sh grabmode DOS/clkprobe.exe

stm.exe: $(STM_DOS_OBJECTS) $(XFREELIBS)
	$(CC) $(CFLAGS) $(STM_DOS_OBJECTS) $(XFREELINK) -o DOS/stm.exe

clkprog.exe: $(CP_OBJECTS) $(XFREELIBS)
	$(CC) $(CFLAGS) $(CP_OBJECTS) $(XFREELINK) -o DOS/clkprog.exe

setpal.exe: $(PAL_DOS_OBJECTS)
	$(CC) $(CFLAGS) $(PAL_DOS_OBJECTS) -o DOS/setpal.exe

getpal.exe: setpal.exe
	DOS/DOSln.sh setpal DOS/getpal.exe

setvgarg.exe: $(VGAREG_OBJECTS)
	$(CC) $(CFLAGS) $(VGAREG_OBJECTS) -o DOS/setvgarg.exe

getvgarg.exe: setvgarg.exe
	DOS/DOSln.sh setvgarg DOS/getvgarg.exe

set80.exe: $(SET80_DOS_OBJECTS)
	$(CC) $(CFLAGS) $(SET80_DOS_OBJECTS) -o DOS/set80.exe

#
# install/clean targets
#

bininstall:
	@if [ ! -x SVGATextMode ]; then \
		make SVGATextMode; \
	fi
	@if [ ! -x grabmode ]; then \
		make grabmode; \
	fi
	install -m 755 SVGATextMode $(INSTBINDIR)
	ln -sf SVGATextMode $(INSTBINDIR)/stm
	install -m 755 grabmode $(INSTBINDIR)
	ln -sf grabmode $(INSTBINDIR)/clockprobe
	
man-install maninstall:
	rm -f $(INSTMANDIR)/man8/SVGATextMode.8.gz
	rm -f $(INSTMANDIR)/man8/stm.8.gz
	rm -f $(INSTMANDIR)/man5/TextConfig.5.gz
	rm -f $(INSTMANDIR)/man8/grabmode.8.gz
	rm -f $(INSTMANDIR)/man8/clockprobe.8.gz
	install -m 644 doc/SVGATextMode.man $(INSTMANDIR)/man8/SVGATextMode.8
	install -m 644 doc/TextConfig.man $(INSTMANDIR)/man5/TextConfig.5
	install -m 644 doc/grabmode.man $(INSTMANDIR)/man8/grabmode.8
	ln -sf grabmode.8 $(INSTMANDIR)/man8/clockprobe.8
	ln -sf SVGATextMode.8 $(INSTMANDIR)/man8/stm.8
	
mangz-install mangzinstall compressedmaninstall: man-install
	gzip -v -f -9 $(INSTMANDIR)/man8/SVGATextMode.8
	gzip -v -f -9 $(INSTMANDIR)/man5/TextConfig.5
	gzip -v -f -9 $(INSTMANDIR)/man8/grabmode.8
	ln -sf grabmode.8.gz $(INSTMANDIR)/man8/clockprobe.8.gz
	ln -sf SVGATextMode.8.gz $(INSTMANDIR)/man8/stm.8.gz
	rm -f $(INSTMANDIR)/man8/clockprobe.8
	rm -f $(INSTMANDIR)/man8/stm.8
	
install: bininstall
	@echo
	@if [ ! -f ${CONFIGFILE} ]; then \
		echo "NOTE:" \
		echo "	You must also have an \`$(CONFIGFILE)' file."; \
		echo "	There's an example of such a file in the main SVGATextMode directory."; \
		echo "	(or type \`make newinstall' to install the TextConfig file as well)"; \
	else \
		contrib/scripts/check_TextConf_version $(CONFIGFILE) $(VERSION_NUM); \
	fi
	@echo
	@echo "NOTE:"
	@echo "	To install the manual pages, type \`make man-install',"
	@echo "	or \`make mangz-install' to install compressed man-pages."
	@echo

newinstall: bininstall	
	@if [ -f ${CONFIGFILE} ]; then \
		echo "backing up previous $(CONFIGFILE) to $(CONFIGFILE).orig"; \
		cp ${CONFIGFILE} ${CONFIGFILE}.orig; \
	fi
	install -m 644 TextConfig $(CONFIGFILE)


backup:
	( cd ..; tar cvzf SVGATextMode-${VERSION_NUM}.tar.gz $(shell basename `pwd`); cd $(shell pwd))


clean:
	$(RM) *~ *.o *.bak *.orig *.rej
	$(MAKE) -C XFREE clean

pristine: mrproper

dosclean: clean     # Leaves yacc/lex files in place since I don't have those for DOS
	$(RM) -f $(BINARIES)
	(cd DOS; $(RM) -f $(DOSPROGS) $(DOSHACKPROGS))
	$(MAKE) -C XFREE mrproper

distclean: clean
	$(RM) -f $(HACKPROGS) DEADJOE  y.tab.h cfglex.c y.tab.c
	$(RM) -f `find . -name '*~' -or -name '*.orig' -or -name '*.rej'`

mrproper: distclean
	$(RM) -f $(BINARIES)
	(cd DOS; $(RM) -f $(DOSPROGS) $(DOSHACKPROGS))
	$(MAKE) -C XFREE mrproper


dist: bindist dosdist srcdist


bindist: mrproper user
	@if [ ! -d ../$(BINDIR) ]; then \
		mkdir ../$(BINDIR); \
	fi
	cp -a ${USERPROGS} COPYING CREDITS Changelog HISTORY INSTALL \
	  README README.FIRST STMmenu TextConfig Makefile INDEX \
	  ../$(BINDIR)
	cp -ar doc ../$(BINDIR)
	mkdir ../$(BINDIR)/contrib
	( cd contrib ; \
	  cp -ar README setfont scripts ../../$(BINDIR)/contrib; \
	  cd ..; \
	)
	touch ../$(BINDIR)/.depend
	( pushd . ; \
	  cd .. ; \
	  tar czf $(BINDIR).tar.gz $(BINDIR); \
	  popd ; \
	)

srcdist: mrproper
	@if [ ! -d ../$(SRCDIR) ]; then \
		mkdir ../$(SRCDIR); \
	fi
	cp -a . ../$(SRCDIR)
	rm -f ../$(SRCDIR)/contrib/DOS/*
	echo "The DOS programs (stm.exe, grabmode.exe, grabwin.exe, scanmode.exe)" >../$(SRCDIR)/contrib/DOS/README
	echo "have been moved to the DOS distribution '"$(DOSDIR)".tar.gz'" >>../$(SRCDIR)/contrib/DOS/README
	( pushd . ; \
	  cd .. ; \
	  tar czf $(SRCDIR).tar.gz $(SRCDIR); \
	  popd ; \
	)

dosdist: alldos
	upx --best DOS/grabmode.exe DOS/stm.exe DOS/clkprog.exe DOS/setvgarg.exe DOS/setpal.exe DOS/set80.exe
	@if [ ! -d ../$(DOSDIR) ]; then \
		mkdir ../$(DOSDIR); \
	fi
	cp -a COPYING README DOS/*.exe ../$(DOSDIR)
	cp -a README.FIRST ../$(DOSDIR)/READ1ST.TXT
	cp -a INSTALL-DOS ../$(DOSDIR)/INSTALL.TXT
	cp -a doc/FAQ ../$(DOSDIR)/FAQ.TXT
	cp -a doc/DOS_programs.doc ../$(DOSDIR)/DOSPROGS.TXT
	cp -a doc/NO_SUPPORT ../$(DOSDIR)/NOSUPORT.TXT
	cp -a doc/PROBLEMS_QuickStart ../$(DOSDIR)/PROBLEMS.TXT
	cp -a doc/README.ET4000 ../$(DOSDIR)/ET4000.TXT
	cp -a doc/README.ET4000.AltClockSelect ../$(DOSDIR)/ET4000-2.TXT
	cp -a doc/creating_textmodes_from_scratch.HOWTO ../$(DOSDIR)/CREATING.TXT
	cp -a doc/grabmode_pixmux ../$(DOSDIR)/GRABMODE.TXT
	cp -a doc/monitor-timings.howto ../$(DOSDIR)/MONITORS.TXT
	cp -a doc/set80.doc ../$(DOSDIR)/set80.txt
	cp -a doc/setVGAreg.doc ../$(DOSDIR)/setvgarg.txt
	cp -a doc/setpalette.doc ../$(DOSDIR)/setpal.txt
	cp -a doc/vgaset.note ../$(DOSDIR)/VGASET.TXT
	patch -o ../$(DOSDIR)/STMText.cfg < TextConfig_DOS.diff
	recode ..pc ../$(DOSDIR)/*.TXT ../$(DOSDIR)/README ../$(DOSDIR)/COPYING ../$(DOSDIR)/STMText.cfg
	groff -Tascii -man doc/SVGATextMode.man | \
	  perl -pe "s/.\010//g;s/\r//g;s/\n/\r\n/g" > ../$(DOSDIR)/stm.man
	groff -Tascii -man doc/grabmode.man | \
	  perl -pe "s/.\010//g;s/\r//g;s/\n/\r\n/g" > ../$(DOSDIR)/grabmode.man
	groff -Tascii -man doc/TextConfig.man | \
	  perl -pe "s/.\010//g;s/\r//g;s/\n/\r\n/g" > ../$(DOSDIR)/STMText.man
	( pushd . ; \
	  cd ../$(DOSDIR) ; \
	  tar czf ../$(DOSDIR).tar.gz *; \
	  popd ; \
	)
	

#
# dependencies
#

depend dep: .depend

.depend::
	gcc -MM *.c >.depend
	$(MAKE) -C XFREE depend


include .depend


