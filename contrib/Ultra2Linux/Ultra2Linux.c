#include <stdio.h>
#include <stdlib.h>

/* Copyright (c) 1995, Kenneth Albanowski */

char buffer[64000];

int main(int argc,char*argv[]) {
        int w,h;
        unsigned char pos;

        if(argc<2) {
                fprintf(stderr,
" Convert between UltraVision fonts and linux fonts

Usage: %s height width < uvfont > linuxfont

For example: %s 20 9 < newfont.f20 > newfont

If you are going to be displaying the font as a 9-pixel wide font, be sure
so give a width of 9 so that any 9-bit data will be recognized and used.

Also note: Fonts included with UltraVision or otherwise provided by 
Personics, Inc. (The makers of UltraVision.) are copyrighted, and may not
be distributed in any form.

This software is provided only for the purpose of converting UltraVision 
fonts for your own personal use.

",argv[0],argv[0]);
                exit(0);
        }

        h=atoi(argv[1]);
        w=atoi(argv[2]);

        fread(buffer,1,h*256,stdin);

        fprintf(stderr,"Read 256 characters of %d bytes each.\n",h);

        if(w!=9)
                goto done;

        while(1) {
                fread(&pos,1,1,stdin);
                if(feof(stdin) || ferror(stdin))
                        goto done;
                fprintf(stderr,"Reading 9-bit character %d.\n",pos);
                fread(&buffer[pos*h],1,h,stdin);
        }


        done:
        fwrite(buffer,1,h*256,stdout);

        fprintf(stderr,"Done.\n");

	exit(0);
}

