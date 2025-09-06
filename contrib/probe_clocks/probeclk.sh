#!/bin/sh
# 
# This script will do some sort of clock probe, like the one the XFREE86
# server does. The difference is that this uses the (more reliable) clockprobe
# from SVGAtextMode instead of the probe from X (see
# doc/README.ET4000.AltClockSelect for more information on this subject)
# 
# It is much slower than the XFREE clock probe! But it probes the clocks AS
# SVGATextMode SEES THEM. So if you suspect the clock ordering in SVGAtextMode
# is different than that from the XFREE code, or if you don't know your pixel
# clocks, this script will tell you what they are.
# 
# If necessary, adjust the script to reflect a different number of clocks (16
# is most common)
#
# WARNING: Don't use this on cards with a clock chip (including Cirrus)
#          They will just create the fake clocks (20, 21, 22, ...) for you!
#

LOG=/tmp/clock_probe.out

echo 'This script will probe your clocks.'
echo 'This is inherently DANGEROUS to use.'
echo 'Press <CTRL-C> if you feel insecure!'
echo 'Or <Enter> when you feel like taking a risk :-)'

read bummer

echo Clock probing output | tee $LOG
echo | tee -a $LOG

for i in  20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
do
        echo | tee -a $LOG
	echo mode: $i | tee -a $LOG
	../../SVGATextMode -t ./TextConfig_probe $i  2>&1 | tee -a $LOG
	../../grabmode 2>&1 | tee -a $LOG 
done
echo -en '\a'; sleep 1; echo -en '\a'; sleep 1 ; echo -en '\a'
../../SVGATextMode 80x25



