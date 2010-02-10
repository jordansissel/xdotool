PREFIX?=/usr/local
INSTALLBIN?=$(PREFIX)/bin
INSTALLLIB?=$(PREFIX)/lib
INSTALLMAN?=$(PREFIX)/man
INSTALLINCLUDE?=$(PREFIX)/include

MAJOR=$(shell sh version.sh --major)
VERSION=$(shell sh version.sh)

WARNFLAGS+=-pedantic -Wall -W -Wundef \
           -Wendif-labels -Wshadow -Wpointer-arith -Wbad-function-cast \
           -Wcast-align -Wwrite-strings -Wstrict-prototypes \
           -Wmissing-prototypes -Wnested-externs -Winline \
           -Wdisabled-optimization -Wno-missing-field-initializers

CFLAGS?=-pipe $(WARNFLAGS)

DEFAULT_LIBS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXtst
DEFAULT_INC=-I/usr/X11R6/include -I/usr/local/include

LIBS=$(shell pkg-config --libs x11 xtst 2> /dev/null || echo "$(DEFAULT_LIBS)")
INC=$(shell pkg-config --cflags x11 xtst 2> /dev/null || echo "$(DEFAULT_INC)")

CFLAGS+=-std=c99 $(INC)
LDFLAGS+=$(LIBS)

all: xdotool xdotool.1 libxdo.so libxdo.so.$(MAJOR)

install: installlib installprog installman installheader

installprog: xdotool
	[ -d $(INSTALLBIN) ] || mkdir -p $(INSTALLBIN)
	install -m 755 xdotool $(INSTALLBIN)/

installlib: libxdo.so
	[ -d $(INSTALLLIB) ] || mkdir -p $(INSTALLLIB)
	install libxdo.so $(INSTALLLIB)/libxdo.so.$(MAJOR)
	ln -sf libxdo.so.$(MAJOR) $(INSTALLLIB)/libxdo.so

installheader: xdo.h
	[ -d $(INSTALLINCLUDE) ] || mkdir -p $(INSTALLINCLUDE)
	install xdo.h $(INSTALLINCLUDE)/xdo.h

installman: xdotool.1
	[ -d $(INSTALLMAN)/man1 ] || mkdir -p $(INSTALLMAN)/man1
	install -m 644 xdotool.1 $(INSTALLMAN)/man1/

deinstall: uninstall
uninstall: 
	rm -f $(INSTALLBIN)/xdotool
	rm -f $(INSTALLMAN)/man1/xdotool.1
	rm -f $(INSTALLLIB)/libxdo.so
	rm -f $(INSTALLLIB)/libxdo.so.$(MAJOR)

clean:
	rm -f *.o xdotool xdotool.1 libxdo.so libxdo.so.$(MAJOR) || true

xdo.o: xdo.c xdo_version.h
	$(CC) $(CFLAGS) -fPIC -c xdo.c

xdotool.o: xdotool.c xdo_version.h
	$(CC) $(CFLAGS) -c xdotool.c

xdo.c: xdo.h
xdotool.c: xdo.h

libxdo.so: xdo.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname=libxdo.so.$(MAJOR) $< -o $@

libxdo.so.$(MAJOR): libxdo.so
	ln -s $< $@

xdotool: xdotool.o libxdo.so
	$(CC) -o $@ xdotool.o -L. -lxdo $(LDFLAGS) 

xdotool.1: xdotool.pod
	pod2man -c "" -r "" xdotool.pod > $@

package: test-package-build create-package

test: xdotool libxdo.so.$(MAJOR)
	cd t/; sh run.sh

xdo_version.h:
	sh version.sh --header > $@

VERSION:
	sh version.sh --shell > $@

pre-create-package:
	rm -f VERSION xdo_version.h

create-package: pre-create-package VERSION xdo_version.h
	@NAME=xdotool-$(VERSION); \
	echo "Creating package: $$NAME"; \
	mkdir $${NAME}; \
	rsync --exclude .svn -a `ls -d *.pod COPYRIGHT *.c *.h examples t CHANGELIST README Makefile* version.sh VERSION 2> /dev/null` $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -rf $${NAME}/
	rm VERSION

# Make sure the package we're building compiles.
test-package-build: create-package
	@NAME=xdotool-$(VERSION); \
	echo "Testing package $$NAME"; \
	tar -zxf $${NAME}.tar.gz; \
	make -C $${NAME} xdotool; \
	rm -rf $${NAME}/
	rm -f $${NAME}.tar.gz

