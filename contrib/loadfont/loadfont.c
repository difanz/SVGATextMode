/*
 * loadfont.c (maybe), a Linux console font loader by Albert Cahalan
 * albert@ccs.neu.edu (also adc@coe.neu.edu & acahalan@lynx.neu.edu)
 * Based on setfont.c by Eugene Crosser & aeb.
 *
 * Version 0.1
 *
 * This file will be part of the fe font editor package.
 * Copyright 1995 Albert Cahalan; all rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

/*
 * This program should read these fonts:
 * Raw files with no header
 * .psf files with 4-byte header
 * .cpi multifont files, like those included with DOS
 * .cp files with 1 font in several sizes
 * *** Font must be 8x6 - 8x32 with 256 chars ***
 */

/*
 * Help!
 * How do I convert "Code Page 161", "Latin 1", "France", and "ISO 03" ?
 */

#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#ifndef bool
#define bool int
#endif

typedef struct _font {
  char data[8192];     /* font data with characters aligned to 32 bytes */
  char name[1024];     /* full pathname to font file */
  int offset;          /* where in the file, -1 if size changed */
  int unit;            /* how tall */
  bool was_modified;   /* for the editor */
  bool readonly;       /* for the editor */
  short codepage;      /* what int'l codepage */
} FONT;

char *progname;

int error;    /* there was an error */
int dbl;      /* true if font gets doubled */
int viewonly; /* true if only seeking info */
int dumping;  /* true if dumping to disk */
int cp;       /* requested code page */
char *ifil;   /* name of input file */
char *ofil;   /* name of output file */
FILE *fp;     /* input file */
int iunit;    /* requested character height */


extern int opterr, optind;
extern char *optarg;

int  dumpfont(FONT *fnt);
int  filesize(int fd);
bool findfile(char name[], char *reqname);
int  guessunits(char *buf);
void init(void);
bool openfont(char *name, char *reqname);
void readfont(FONT *fnt);
int  savefont(FONT *fnt);
int  search_cp(void);
int  seekfont(int iunit);
int  uploadfont(char *readytogo);
void usage(char *msg);

/*****************************************************/
int main(int argc, char **argv){
  FONT *stdfont;
  int i;

  progname = argv[0];
  init();
  while ((i = getopt(argc, argv, "c:s:dv2")) != -1){
    switch(i){
      case 'c':
        if(cp==0) cp = atoi(optarg);
        else usage("Make up your mind, what code page?");
        if(cp==0) usage("Code page 0 causes internal errors!");
	break;
      case 's':
        if(iunit==0) iunit = atoi(optarg);
        else usage("Make up your mind, what size?");
        if(iunit < 6 || iunit > 32) usage("Insane font size!");
	break;
      case 'd':
        if(!dumping) dumping = TRUE;
        else usage("Can't take a dump twice.");
	break;
      case 'p':
        if(!dumping) dumping = 2;  /* .psf dump */
        else usage("Can't take a dump twice.");
	break;
      case '2':
        if(!dbl) dbl = TRUE;
        else usage("Already doubled. Already doubled.");
	break;
      case 'v':
        if(!viewonly) viewonly = TRUE;
        else usage("Already in viewonly mode.");
	break;
      case '?': usage("Bad option, you need help.");
      default: usage("Internal error:  getopt returned unknown token.");
    }
  }

  /* Check parameters for sanity */
  if(dbl && (iunit>16)) fprintf(stderr,"Font too big, was truncuated.\n");
  if(optind==argc){ /* no file */
    if(dumping) usage("Dump font where?");
    else ifil="";
  }
  if(optind+1==argc){ /* one file */
    if(dumping) ofil=argv[optind];
    else ifil=argv[optind];
  }
  if(optind+2==argc){ /* two files */
    if(dumping){ /* dump then load */
      usage("Dump what?");
    }else{ /* load then dump w/o ioctl (convert) */
      ifil=argv[optind];
      ofil=argv[optind+1];
    }
  }
  if(optind+2<argc){ /* too many files */
    usage("Way too many parameters...");
  }
  if(!ifil && !ofil) usage("Oh, fuck!"); /* This should never happen... */

  if (ofil && dumping){
    stdfont = calloc(1,sizeof(FONT));
    stdfont->unit=iunit;
    strncpy(stdfont->name,ofil,1023);
    if(dumpfont(stdfont)==0) savefont(stdfont);
    free(stdfont);
    ofil=0;
  }

  if(ifil) {        /* Get a font from disk. */
    stdfont = calloc(1,sizeof(FONT));
    if(!openfont(stdfont->name,ifil)) return 1;
    stdfont->unit = seekfont(iunit);
    if(!viewonly) fprintf(stderr, "Units are %u.\n",(unsigned char)(stdfont->unit));
    if((stdfont->unit>5) && (stdfont->unit<33)){
      if(!viewonly) readfont(stdfont);
      fclose(fp);
      if(ofil && !viewonly){ /* conversion only */
        strncpy(stdfont->name,ofil,1023);
        savefont(stdfont);
      }else{ /* send font to kernal */
        uploadfont(stdfont->data);
      }
    }else if(!viewonly) {
      fclose(fp);
      fprintf(stderr,"Bad font size %i\n",stdfont->unit);
      return(1);
    }
    free(stdfont);
  }

  return(0);
}
/*****************************************************/
void init(void){
  ifil = 0;
  ofil = 0;
  iunit = 0;
  error = FALSE;
  dbl = FALSE;
  dumping = FALSE;
  viewonly = FALSE;
  cp = 0;
  fp = (FILE *)0xdeadbeef; /* cause an error if file not opened */
}
/*****************************************************/
/*********************************************/
int dumpfont(FONT *fnt) {
  int fd;
  /* we do not want to read, but only need fd */
  if ((fd=open("/dev/console",0)) < 0) fd=0;
  if(ioctl(fd,GIO_FONT,fnt->data)) {
    perror("GIO_FONT ioctl error");
    close(fd);
    return(1);
  }
  close(fd);
  if (fnt->unit == 0) fnt->unit=guessunits(fnt->data);
  if (fnt->unit < 6){
    fprintf(stderr, "Found nothing big enough to save.\n");
    return(1);
  }
  return(0);
}
/*********************************************/
int savefont(FONT *fnt) {
	int i;  
        if((fp = fopen(fnt->name, "r+")) == NULL){   /* read,write */
          if((fp = fopen(fnt->name, "w")) == NULL){  /* create the file */
	    perror(fnt->name);
	    return(-1);
	  }
	} else {
	  if(seekfont(fnt->unit) != fnt->unit){
	    fprintf(stderr, "Wrong font size.\n");
            fclose(fp);
	    return(-1);
	  }
	}
	for (i = 0; i < 256; i++)
	  if (fwrite(fnt->data+(32*i), (unsigned int)fnt->unit, 1, fp) != 1) {
	  perror("Cannot write font file");
	  return(-1);
        }
	printf("Saved 8x%d font in %s\n", fnt->unit, fnt->name);
	fclose(fp);
	return 0;
}
/*********************************************/
int guessunits(char *buf){ /* save font as efficiently as possible */
  int i;
  int unit;
  for (unit = 32; unit > 0; unit--)
    for (i = 0; i < 256; i++)
      if (buf[32*i+unit-1]) return unit;
  return 0; /* keep compiler happy */
}
/*********************************************/
void readfont(FONT *fnt){
  int i;
  if(dbl){
    for (i=0; i<256; i++)
      if (fread(fnt->data+(16*i), (unsigned int)fnt->unit, 1, fp) != 1) { /* Note the 16 */
        perror ("Cannot read font from file");
	exit (1);
      }
    for (i=4095; i>=0; i--){ /* Time to double the font */
      *(fnt->data+(i*2))   = *(fnt->data+i);
      *(fnt->data+(i*2+1)) = *(fnt->data+i);
    }
    fnt->unit=fnt->unit*2; /* store real size in FONT struct */
    if(fnt->unit>32){
      fprintf(stderr,"Font too big, was truncuated.\n");
      fnt->unit=32;
    }
    fnt->was_modified=TRUE; /* we can't put this back where it came from */
  }else{
    for (i=0; i<256; i++)
      if (fread(fnt->data+(32*i), (unsigned int)fnt->unit, 1, fp) != 1) { /* Note the 32 */
        perror ("Cannot read font from file");
	exit (1);
      }
  }
}
/*********************************************/
void usage(char *msg){
  fprintf(stderr, "Error:  %s\n", msg);
  fprintf(stderr, "\
Usage:  %s [-d] [-2] [-v] [-c codepage] [-s fontsize] font1 [font2]
If no -d is given, and no font, then the font \"default\" is loaded.
With two fonts a conversion is done.
Options:
 -v		View info only.
 -2		Double the height as a font is read in.
 -d		Dump current font to a file.
 -c codepage	Select a codepage from a multi-font file.
 -s fontsize	Select a font size from a multi-font file.
", progname);
  exit(1);
}
/*********************************************/
int seekfont(int iunit){
    int hdr;
    int size;

    size=filesize(fp->_fileno);
    if(size==0){
      return(0);  /* nothing there, may be valid if writing */
    }
    
    if (size == 9780) {  /* This is a codepage file */
	/* normal code page: first 34 bytes main header,
	   then 8x16, 8x14, and 8x8 fonts each preceded by 6 byte subheader */
	return(search_cp());
    }

    if((size>9780) && (size<65537)){  /* This is a big file, might be .cpi */
    	int cpnum;
    	char cpihdr[25];
    	char cpimagic[] = "\377FONT   ";
	if (fread(cpihdr, 25, 1, fp) != 1) {
	    perror("Error reading input font .cpi header");
	    return(-1);
	}
	if(strncmp(cpimagic,cpihdr,8) != 0){ /* What the hell? */
	  fprintf(stderr, "Unknown font file type.\n");
	  return(-1);
	}
	cpnum=cpihdr[23]; /* number of .cp files inside .cpi */
	fprintf(stderr, "%i code pages in this .cpi file.\n",cpnum);
	/* start a loop here checking each .cp part for a cp/unit match */
	while(cpnum){
	  int tmpunits;
	  tmpunits=search_cp();
	  if(tmpunits && !viewonly) return(tmpunits);
	  cpnum--;
	}
	if(!viewonly){
	  fprintf(stderr, ".cpi file did not contain requested font.\n");
	  return(-1);
	}
	return(0);
    }
    
    /* Not a normal codepage file, so do basic error checking & setup. */
    if (iunit) {
        fprintf(stderr, "This file contains only 1 font.\n");
        return(-1);
    }
    hdr = (size & 255);
    iunit = (size >>8);
    if (iunit < 6 || iunit > 32) {
        /* Although font sizes less than 6 are valid, they are stupid. */
	fprintf(stderr, "Bad input file size.\n");
	return(-1);
    }

    if (hdr == 4) {                       /* .psf file */
	char psfhdr[4];
	if (fread(psfhdr, 4, 1, fp) != 1) {
	    perror("Error reading header input font");
	    return(-1);
	}
	/* note: this depends on endianness */
	if (psfhdr[1] != 0x04 || psfhdr[0] != 0x36) {
	    fprintf(stderr, "Unrecognized font format\n");
	    return(-1);
	}
	if (psfhdr[2] != 0) {
	    fprintf(stderr, "Unsupported psf file mode\n");
	    return(-1);
	}
	if(size != hdr + 256*psfhdr[3]) {
	    fprintf(stderr, "Input file: bad length\n");
	    return(-1);
	}
	return(iunit);
    }
    
    if (hdr == 0) return(iunit);  /* must be raw file, needs no work */
    
    if (hdr == 40) {                       /* .cp file with only one font */
	return(search_cp());
    }
    
    /* add more file types here */
    fprintf(stderr, "Unknown input file type.\n");
    return(-1);
}
/*********************************************/
int search_cp(void){ /* return num scanlines, discard cp number? */
  char hdr[34];
  unsigned short numfonts;
  unsigned short fontsleft;
  unsigned short thispage;
  bool skip;
  unsigned short *sptr;
  sptr=(short *)hdr;
  if(fread(hdr, 34, 1, fp) != 1) {
    perror("Error reading .cp header");
    return(0);
  }
  numfonts=sptr[15];
  skip=FALSE;
  thispage=sptr[8];
  if(viewonly){
    fprintf(stderr,"Code page %u with %u fonts.\n",thispage,numfonts);
  }else{
    if( (cp!=0) && (cp!=thispage) ){
      skip=TRUE;/* seek to next cp */
    }else{
      fprintf(stderr,"Code page %u found.\n",thispage);
    }
  }
  fontsleft=numfonts;
  while(fontsleft--){
    if(fread(hdr, 6, 1, fp) != 1) {
      perror("Error reading .cp subheader");
      return(0);
    }
    if(viewonly) fprintf(stderr,"%ux%u ",hdr[1],hdr[0]);
    if((hdr[1]==8)&&(!viewonly)&&(!skip)){
      if((iunit==0)&&(numfonts==1)) return(hdr[0]); /* didn't care, this will do */
      if(hdr[0]==iunit) return(iunit);
    }
    fseek(fp,256*hdr[0],SEEK_CUR);/* seek to end of font */
  }
  if((cp==thispage)&&(!viewonly)) fprintf(stderr,"Multiple sizes, which one?");
  fprintf(stderr,"\n");
  return(0);
}
/*****************************************************/
/* find input file; leave name in pathname[] */
bool findfile(char name[], char *reqname) {
  char *dirpath[] = { "", "/usr/lib/kbd/consolefonts/", 0 };
  char *suffixes[] = { "", ".psf", ".cp", ".cpi", ".fnt", 0 }; /* fot fon ttf */
  char **dp, **sp;

  for (dp = dirpath; *dp; dp++) {
    if (*reqname == '/' && **dp) continue;
    for (sp = suffixes; *sp; sp++) {
      if (strlen(*dp)+strlen(reqname)+strlen(*sp)+1 > 1024) continue;
      /* name will not be bigger than allowed, 1024 chars */
      sprintf(name, "%s%s%s", *dp, reqname, *sp);
      if((fp=fopen(name, "r")) != NULL) return TRUE;
    }
  }
  return FALSE;
}
/*********************************************/
/*********************************************/
bool openfont(char *name, char *reqname){
    char defname[20];
    if (*reqname) {  /* a specific file was requested */
	if (!findfile(name, reqname)) {
	    fprintf(stderr, "Cannot open font file %s.\n", reqname);
	    return FALSE;
	} else return TRUE;
    } else {   /* try to find some default file */
        sprintf(defname, "%i", cp);
	if (findfile(name,defname)) return TRUE;
        sprintf(defname, "default8x%i", iunit);
	if (findfile(name,defname)) return TRUE;
        sprintf(defname, "8x%i", iunit);
	if (findfile(name,defname)) return TRUE;
	
	if (findfile(name,"default")) return TRUE;
	if (findfile(name,"8x8plus")) return TRUE;
        fprintf(stderr, "Cannot find default font.\n");
	return FALSE;
    }
}
/*********************************************/
int filesize(int fd){
  /* pass a file descriptor (maybe fp->_fileno, fp is FILE*), get the size. */
  struct stat stbuf;
  if (fstat(fd, &stbuf)) {
    perror("Couldn't stat file");
    exit(1);
  }
  return(stbuf.st_size);
}

/*********************************************/
int uploadfont(char *readytogo){ /* give font to kernal for display */
	int fd;
	if(viewonly){
	  fprintf(stderr,"Pretended to upload font to kernal.\n");
	  return(0);
	}
	/* we do not want to read, but only need fd */
	if ((fd=open("/dev/console",0)) < 0) fd=0;
	if(ioctl(fd,PIO_FONT,readytogo)) {
	  perror("PIO_FONT ioctl error");
	  close(fd);
	  return(1);
	} else fprintf(stderr, "Uploaded font to kernal.\n");
	close(fd);
	return(0);
}
/*********************************************/
