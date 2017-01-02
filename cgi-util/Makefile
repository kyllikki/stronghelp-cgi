# Makefile for cgi-util example

# by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/cgi-util/

# June 12, 1999 - May 16, 2004


# Makefile options:

CC=gcc
CFLAGS=-g -Wall -ansi -pedantic
#-Werror
LIBDIR=/usr/local/lib
INCDIR=/usr/local/include


# Makefile "commands":

all:	default

default:	libcgi-util.so libcgi-util.a test-cgis
	@echo
	@echo "cgi-util make options:"
	@echo "----------------------"
	@echo "install    - install everything"
	@echo "install-a  - install the libcgi-util.a static library"
	@echo "install-so - install the libcgi-util.so shared library"
	@echo "test-cgis  - compile test.cgi, filetest.cgi (using cgi-util.o)"
	@echo

install:	install-h install-a install-so

install-h:	cgi-util.h
	cp cgi-util.h $(INCDIR)/
	chmod 644 $(INCDIR)/cgi-util.h

install-a:	libcgi-util.a install-h
	cp libcgi-util.a $(LIBDIR)/
	chmod 644 $(LIBDIR)/libcgi-util.a

install-so:	libcgi-util.so install-h
	cp libcgi-util.so $(LIBDIR)/
	chmod 644 $(LIBDIR)/libcgi-util.so

test-cgis:	test.cgi filetest.cgi

clean:
	-rm -f core cgi-util.o libcgi-util.a libcgi-util.so libcgi-util.o
	-rm -f test.o test.cgi filetest.o filetest.cgi


# The test CGIs:

test.cgi:	test.o cgi-util.o
	$(CC) $(CFLAGS) test.o cgi-util.o -o test.cgi
	chmod 755 test.cgi
	chmod 644 test.html

filetest.cgi:	filetest.o cgi-util.o
	$(CC) $(CFLAGS) filetest.o cgi-util.o -o filetest.cgi
	chmod 755 filetest.cgi
	chmod 644 filetest.html


# Test CGIs Objects:

test.o:	test.c cgi-util.h

filetest.o:	filetest.c cgi-util.h


# Objects:

cgi-util.o:	cgi-util.c cgi-util.h

libcgi-util.so:
	$(CC) $(CFLAGS) -fPIC cgi-util.c -c -o libcgi-util.o
	$(CC) -shared libcgi-util.o -o libcgi-util.so

libcgi-util.a:	cgi-util.o
	ar rcs libcgi-util.a cgi-util.o
