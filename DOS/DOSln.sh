#!/bin/bash

#Make DJGPP-style symlinks (Mini-Stubs calling another program)
#$1 must not have a path or an extension (.exe)!
stubify -g $2
stubedit $2 runfile=$1
chmod 755 $2
