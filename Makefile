DESTDIR?=
PREFIX?=/usr/local
INSTALLBIN?=$(PREFIX)/bin
INSTALLLIB?=$(PREFIX)/lib
INSTALLMAN?=$(PREFIX)/man
INSTALLINCLUDE?=$(PREFIX)/include

DPREFIX=$(DESTDIR)$(PREFIX)
DINSTALLBIN=$(DESTDIR)$(INSTALLBIN)
DINSTALLLIB=$(DESTDIR)$(INSTALLLIB)
DINSTALLMAN=$(DESTDIR)$(INSTALLMAN)
DINSTALLINCLUDE=$(DESTDIR)$(INSTALLINCLUDE)

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

all: xdotool xdotool.1 libxdo.so libxdo.so.$(MAJOR)

install: pre-install installlib installprog installman installheader

pre-install:
	install -d $(DPREFIX)

installprog: xdotool
	install -d $(DINSTALLBIN)
	install -m 755 xdotool $(DINSTALLBIN)/

installlib: libxdo.so
	install -d $(DINSTALLLIB)
	install libxdo.so $(DINSTALLLIB)/libxdo.so.$(MAJOR)
	ln -sf libxdo.so.$(MAJOR) $(DINSTALLLIB)/libxdo.so

installheader: xdo.h
	install -d $(DINSTALLINCLUDE)
	install xdo.h $(DINSTALLINCLUDE)/xdo.h

installman: xdotool.1
	install -d $(DINSTALLMAN)/man1
	install -m 644 xdotool.1 $(DINSTALLMAN)/man1/

deinstall: uninstall
uninstall: 
	rm -f $(DINSTALLBIN)/xdotool
	rm -f $(DINSTALLMAN)/xdotool.1
	rm -f $(DINSTALLLIB)/libxdo.so
	rm -f $(DINSTALLLIB)/libxdo.so.$(MAJOR)

clean:
	rm -f *.o xdotool xdotool.1 libxdo.so libxdo.so.? || true

xdo.o: xdo.c xdo_version.h
	$(CC) $(CFLAGS) -fPIC -c xdo.c

xdo_search.o: xdo_search.c
	$(CC) $(CFLAGS) -fPIC -c xdo_search.c

xdotool.o: xdotool.c xdo_version.h
	$(CC) $(CFLAGS) -c xdotool.c

xdo_search.c: xdo.h
xdo.c: xdo.h
xdotool.c: xdo.h

libxdo.so: xdo.o xdo_search.o
	$(CC) $(LDFLAGS) $(LIBS) -shared -Wl,-soname=libxdo.so.$(MAJOR) xdo.o xdo_search.o -o $@

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

