CGI_OBJ= cgi-util/cgi-util.o
CGI_INC= cgi-util

VERSION= 0.95
CC= gcc
OPT= -O2
DEBUG= -g
CFLAGS= $(OPT) -Wall -I$(CGI_INC)/
LDFLAGS= $(DEBUG) $(OPT)
HFILES= sh-cgi.h shif.h shhtml.h
CFILES= sh-cgi.c shif.c shhtml.c
TARFILES= $(CFILES) $(HFILES) makefile README LICENCE background.png background.gif
TARNAME= sh-cgi-$(VERSION).tar

EXE= sh-cgi
OBJ= sh-cgi.o shif.o shhtml.o $(CGI_OBJ)

.PHONY: all clean tags TAGS

all: $(EXE)

sh-cgi: $(OBJ)

$(OBJ): $(HFILES)

clean: ; -rm -f core $(EXE) $(OBJ) TAGS *~ $(TARNAME) $(TARNAME).gz

tags: TAGS
	TAGS: $(CFILES) $(HFILES)
	etags -t $(CFILES) $(HFILES)

tar: $(TARFILES)
	tar -cf $(TARNAME) $(TARFILES) cgi-util
	gzip $(TARNAME)

