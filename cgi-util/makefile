# makefile for cgi-util example

# by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/cgi-util/

# June 12, 1999 - June 15, 1999


# Makefile options:

CFLAGS=-g -Wall


# Makefile "commands":

default:
	@echo
	@echo "cgi-util make options:"
	@echo "----------------------"
	@echo "install    - install everything"
	@echo "install-o  - install only the cgi-util.o object"
	@echo "install-a  - install the libcgi-util.a static library"
	@echo "install-so - install the libcgi-util.so shared library"
	@echo "test.cgi   - compile the test.cgi (using cgi-util.o)"
	@echo

install:	install-o install-a install-so test.cgi

install-o:	cgi-util.o
	cp cgi-util.o /usr/local/lib/
	chmod 644 /usr/local/lib/cgi-util.o
	cp cgi-util.h /usr/local/include/
	chmod 644 /usr/local/include/cgi-util.h

install-a:	libcgi-util.a
	cp libcgi-util.a /usr/local/lib/
	chmod 644 /usr/local/lib/libcgi-util.a

install-so:	libcgi-util.so
	cp libcgi-util.so /usr/local/lib
	chmod 644 /usr/local/lib/libcgi-util.so

clean:
	-rm cgi-util.o libcgi-util.a libcgi-util.so libcgi-util.o
	-rm test.o test.cgi


# The test CGI:

test.cgi:	test.o cgi-util.o
	$(CC) $(CFLAGS) test.o cgi-util.o -o test.cgi
	chmod 755 test.cgi


# Objects:

test.o:	test.c cgi-util.h

cgi-util.o:	cgi-util.c cgi-util.h

libcgi-util.so:
	$(CC) $(CFLAGS) -fPIC cgi-util.c -c -o libcgi-util.o
	$(CC) -shared libcgi-util.o -o libcgi-util.so

libcgi-util.a:	cgi-util.o
	ar rcs libcgi-util.a cgi-util.o
