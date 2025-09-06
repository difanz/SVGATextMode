#
# Makefile for font dump/undump tools
#
# Author: Walter Gadeyne (gadeynew@sebb.bel.alcatel.be)
#           

CFLAGS = -Wall -O2 -N -s 

all: dump undump

dump: dump.c
	$(CC) $(CFLAGS) dump.c -o dump
	
undump: undump.c
	$(CC) $(CFLAGS) undump.c -o undump

clean:
	$(RM) *~ *.o *.bak *.orig *.rej dump undump

