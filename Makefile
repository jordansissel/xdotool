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
LIBSUFFIX=$(shell sh platform.sh libsuffix)
VERLIBSUFFIX=$(shell sh platform.sh libsuffix $(MAJOR))
DYNLIBFLAG=$(shell sh platform.sh dynlibflag)
LIBNAMEFLAG=$(shell sh platform.sh libnameflag $(MAJOR) $(INSTALLLIB))
 
CFLAGS?=-pipe -O2 $(WARNFLAGS)

DEFAULT_LIBS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXtst
DEFAULT_INC=-I/usr/X11R6/include -I/usr/local/include

LIBS=$(shell pkg-config --libs x11 xtst 2> /dev/null || echo "$(DEFAULT_LIBS)")
INC=$(shell pkg-config --cflags x11 xtst 2> /dev/null || echo "$(DEFAULT_INC)")

CFLAGS+=-std=c99 $(INC)

CMDOBJS= cmd_click.o cmd_mousemove.o cmd_mousemove_relative.o cmd_mousedown.o \
         cmd_mouseup.o cmd_getmouselocation.o cmd_type.o cmd_key.o \
         cmd_windowmove.o cmd_windowactivate.o cmd_windowfocus.o \
         cmd_windowraise.o cmd_windowsize.o cmd_set_window.o cmd_search.o \
         cmd_getwindowfocus.o cmd_getwindowpid.o cmd_getactivewindow.o \
         cmd_windowmap.o cmd_windowunmap.o cmd_set_num_desktops.o \
         cmd_get_num_desktops.o cmd_set_desktop.o cmd_get_desktop.o \
         cmd_set_desktop_for_window.o cmd_get_desktop_for_window.o

.PHONY: all
all: xdotool.1 libxdo.$(LIBSUFFIX) libxdo.$(VERLIBSUFFIX) xdotool

.PHONY: install
install: pre-install installlib installprog installman installheader post-install

.PHONY: pre-install
pre-install:
	install -d $(DPREFIX)

.PHONY: post-install
post-install:
	@if [ "$$(uname)" = "Linux" ] ; then \
		echo "Running ldconfig to update library cache"; \
		ldconfig \
		  || echo "Failed running 'ldconfig'. Maybe you need to be root?"; \
	fi


.PHONY: installprog
installprog: xdotool
	install -d $(DINSTALLBIN)
	install -m 755 xdotool $(DINSTALLBIN)/

.PHONY: installlib
installlib: libxdo.$(LIBSUFFIX)
	install -d $(DINSTALLLIB)
	install libxdo.$(LIBSUFFIX) $(DINSTALLLIB)/libxdo.$(VERLIBSUFFIX)
	ln -sf libxdo.$(VERLIBSUFFIX) $(DINSTALLLIB)/libxdo.$(LIBSUFFIX)

.PHONY: installheader
installheader: xdo.h
	install -d $(DINSTALLINCLUDE)
	install xdo.h $(DINSTALLINCLUDE)/xdo.h

.PHONY: installman
installman: xdotool.1
	install -d $(DINSTALLMAN)/man1
	install -m 644 xdotool.1 $(DINSTALLMAN)/man1/

.PHONY: deinstall
deinstall: uninstall

.PHONY: uninstall
uninstall: 
	rm -f $(DINSTALLBIN)/xdotool
	rm -f $(DINSTALLMAN)/xdotool.1
	rm -f $(DINSTALLLIB)/libxdo.$(LIBSUFFIX)
	rm -f $(DINSTALLLIB)/libxdo.$(VERLIBSUFFIX)

.PHONY: clean
clean:
	rm -f *.o xdotool xdotool.1 xdotool.html libxdo.$(LIBSUFFIX) libxdo.$(VERLIBSUFFIX) || true

xdo.o: xdo.c xdo_version.h
	$(CC) $(CFLAGS) -fPIC -c xdo.c

xdo_search.o: xdo_search.c
	$(CC) $(CFLAGS) -fPIC -c xdo_search.c

xdotool.o: xdotool.c xdo_version.h
	$(CC) $(CFLAGS) -c xdotool.c

xdo_search.c: xdo.h
xdo.c: xdo.h
xdotool.c: xdo.h

libxdo.$(LIBSUFFIX): xdo.o xdo_search.o
	$(CC) $(LDFLAGS) $(DYNLIBFLAG) $(LIBNAMEFLAG) xdo.o xdo_search.o -o $@ $(LIBS)

libxdo.$(VERLIBSUFFIX): libxdo.$(LIBSUFFIX)
	ln -s $< $@

xdotool: xdotool.o $(CMDOBJS) libxdo.$(LIBSUFFIX)
	$(CC) -o $@ xdotool.o $(CMDOBJS) -L. -lxdo $(LDFLAGS)  -lm

xdotool.1: xdotool.pod
	pod2man -c "" -r "" xdotool.pod > $@

showman: xdotool.1
	nroff -man $< | $$PAGER

xdotool.html: xdotool.pod
	pod2html $< > $@

.PHONY: package
package: test-package-build create-package

.PHONY: test
test: xdotool libxdo.$(VERLIBSUFFIX)
	$(MAKE) -C t

xdo_version.h:
	sh version.sh --header > $@

VERSION:
	sh version.sh --shell > $@

.PHONY: pre-create-package
pre-create-package:
	rm -f VERSION xdo_version.h

.PHONY: create-package
create-package: pre-create-package VERSION xdo_version.h
	@NAME=xdotool-$(VERSION); \
	echo "Creating package: $$NAME"; \
	mkdir $${NAME}; \
	rsync --exclude .svn --exclude '.*' -a `ls -d *.pod COPYRIGHT *.c *.h examples t CHANGELIST README Makefile* version.sh platform.sh VERSION 2> /dev/null` $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -r $${NAME}
	rm VERSION

# Make sure the package we're building compiles.
.PHONY: test-package-build
test-package-build: create-package
	@NAME=xdotool-$(VERSION) && \
	echo "Testing package $$NAME" && \
	tar -zxf $${NAME}.tar.gz && \
	make -C $${NAME} test && \
	echo "Package ready: $${NAME}"; \
	rm -rf $${NAME}

