@echo off
del config.err
rem djtar does not make symlinks from tar, so we have to copy files!
        copy xfree\include\xfuncproto.h xfree\include\x11

rem Try DR/Novell/OpenDos to rename directory:
        if direxist DOS goto OpenDos
        if direxist _dos goto go_on
rem If failed, try MS-Dos move:
        move DOS _dos
        goto go_on
:OpenDos
        rendir DOS _dos
:go_on
rem Test if makefile.in exist, if not, make it
        if not exist makefile.in copy makefile makefile.in
rem Make makefile!
echo Configuring makefile for DJGPP
sed -f _dos/configdj.sed makefile.in > makefile
sed -e s/y.tab/y_tab/ cfglex.l > cfglex.dos
sed -e s/.depend/_depend/ XFREE/makefile > XFREE/makefile.dos
