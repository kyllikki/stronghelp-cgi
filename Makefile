CGI_NAME= sh-cgi
VERSION= 1.12

CGI_OBJ= cgi-util/cgi-util.o
CGI_INC= cgi-util

CC= gcc
BUILDDATE=$(shell date +'%e %b %Y')

#Comment for debug
OPT= -O2
#unbcomment for debug
#DEBUG= -g

CFLAGS= $(OPT) $(DEBUG) -Wall -I$(CGI_INC)/ -DVERSION=\""$(VERSION)"\" -DBUILDDATE=\""$(BUILDDATE)"\" -DCGI_NAME=\""$(CGI_NAME)"\"

LDFLAGS= $(DEBUG)
HFILES= sh-cgi.h shif.h shhtml.h
CFILES= sh-cgi.c shif.c shhtml.c
TARFILES= $(CFILES) $(HFILES) Makefile README LICENCE background.png background.gif
TARNAME= sh-cgi-$(VERSION).tar

EXE= $(CGI_NAME)
OBJ= sh-cgi.o shif.o shhtml.o $(CGI_OBJ)

.PHONY: all clean tags TAGS

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ): $(HFILES)

clean: ; -rm -f core $(EXE) $(OBJ) TAGS *~ $(TARNAME) $(TARNAME).gz

tags: TAGS
	TAGS: $(CFILES) $(HFILES)
	etags -t $(CFILES) $(HFILES)

tar: $(TARFILES)
	tar -cf $(TARNAME) $(TARFILES) cgi-util
	gzip $(TARNAME)
