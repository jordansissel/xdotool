DESTDIR?=
PREFIX?=/usr/local
INSTALLBIN?=$(PREFIX)/bin
INSTALLLIB?=$(PREFIX)/lib
INSTALLMAN?=$(PREFIX)/man
INSTALLINCLUDE?=$(PREFIX)/include
INSTALLPC?=$(PREFIX)/lib/pkgconfig
LDCONFIG?=ldconfig

DPREFIX=$(DESTDIR)$(PREFIX)
DINSTALLBIN=$(DESTDIR)$(INSTALLBIN)
DINSTALLLIB=$(DESTDIR)$(INSTALLLIB)
DINSTALLMAN=$(DESTDIR)$(INSTALLMAN)
DINSTALLINCLUDE=$(DESTDIR)$(INSTALLINCLUDE)
DINSTALLPC=$(DESTDIR)$(INSTALLPC)

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
CFLAGS+=-g # TODO(sissel): Comment before release
CFLAGS+=$(CPPFLAGS)
CFLAGS+=$(shell sh cflags.sh)

DEFAULT_LIBS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXtst -lXinerama -lxkbcommon
DEFAULT_INC=-I/usr/X11R6/include -I/usr/local/include

XDOTOOL_LIBS=$(shell pkg-config --libs x11 2> /dev/null || echo "$(DEFAULT_LIBS)")  $(shell sh platform.sh extralibs)
LIBXDO_LIBS=$(shell pkg-config --libs x11 xtst xinerama xkbcommon 2> /dev/null || echo "$(DEFAULT_LIBS)")
INC=$(shell pkg-config --cflags x11 xtst xinerama xkbcommon 2> /dev/null || echo "$(DEFAULT_INC)")
CFLAGS+=-std=c99 $(INC)

CMDOBJS= cmd_click.o cmd_mousemove.o cmd_mousemove_relative.o cmd_mousedown.o \
         cmd_mouseup.o cmd_getmouselocation.o cmd_type.o cmd_key.o \
         cmd_windowmove.o cmd_windowactivate.o cmd_windowfocus.o \
         cmd_windowraise.o cmd_windowsize.o cmd_windowstate.o cmd_set_window.o cmd_search.o \
         cmd_getwindowfocus.o cmd_getwindowpid.o cmd_getactivewindow.o \
         cmd_windowmap.o cmd_windowunmap.o cmd_windowreparent.o \
         cmd_set_num_desktops.o \
         cmd_get_num_desktops.o cmd_set_desktop.o cmd_get_desktop.o \
         cmd_set_desktop_for_window.o cmd_get_desktop_for_window.o \
         cmd_get_desktop_viewport.o cmd_set_desktop_viewport.o \
         cmd_windowkill.o cmd_behave.o cmd_window_select.o \
         cmd_getwindowname.o cmd_getwindowclassname.o cmd_behave_screen_edge.o \
         cmd_windowminimize.o cmd_exec.o cmd_getwindowgeometry.o \
         cmd_windowclose.o cmd_windowquit.o \
         cmd_sleep.o cmd_get_display_geometry.o

.PHONY: all
all: xdotool.1 libxdo.$(LIBSUFFIX) libxdo.$(VERLIBSUFFIX) xdotool

.PHONY: static
static: xdotool.static

.PHONY: install-static
install-static: xdotool.static
	install -d $(DINSTALLBIN)
	install -m 755 xdotool.static $(DINSTALLBIN)/xdotool

xdotool.static: xdotool.o $(CMDOBJS) xdo.o xdo_search.o
	$(CC) -o xdotool.static xdotool.o xdo.o xdo_search.o $(CMDOBJS) $(LDFLAGS)  -lm $(XDOTOOL_LIBS) $(LIBXDO_LIBS)

.PHONY: install
install: pre-install installlib installprog installman installheader installpc post-install

.PHONY: pre-install
pre-install:
	install -d $(DPREFIX)

.PHONY: post-install
post-install:
	@if [ "$$(uname)" = "Linux" ] ; then \
		echo "Running ldconfig to update library cache"; \
		$(LDCONFIG) \
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
	install -m 0644 xdo.h $(DINSTALLINCLUDE)/xdo.h

.PHONY: installpc
installpc: libxdo.pc
	install -d $(DINSTALLPC)
	install libxdo.pc $(DINSTALLPC)/libxdo.pc

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
	rm -f *.o xdotool xdotool.static xdotool.1 xdotool.html xdo_version.h \
	      libxdo.$(LIBSUFFIX) libxdo.$(VERLIBSUFFIX) libxdo.a || true

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
	$(CC) $(LDFLAGS) $(DYNLIBFLAG) $(LIBNAMEFLAG) xdo.o xdo_search.o -o $@ $(LIBXDO_LIBS)

libxdo.a: xdo.o xdo_search.o
	ar qv $@ xdo.o xdo_search.o

libxdo.$(VERLIBSUFFIX): libxdo.$(LIBSUFFIX)
	ln -s $< $@

libxdo.pc:
	sh pc.sh $(VERSION) $(INSTALLLIB) $(INSTALLINCLUDE) > libxdo.pc

# xdotool the binary requires libX11 now for XSelectInput and friends.
# This requirement will go away once more things are refactored into
# libxdo.
# TODO(sissel): only do this linker hack if we're using GCC?
xdotool: LDFLAGS+=-Xlinker
ifneq ($(WITHOUT_RPATH_FIX),1)
xdotool: LDFLAGS+=-rpath $(INSTALLLIB)
endif
xdotool: xdotool.o $(CMDOBJS) libxdo.$(LIBSUFFIX)
	$(CC) -o $@ xdotool.o $(CMDOBJS) -L. -lxdo $(LDFLAGS)  -lm $(XDOTOOL_LIBS)

xdotool.1: xdotool.pod
	pod2man -c "" -r "" xdotool.pod > $@

.PHONY: showman
showman: xdotool.1
	nroff -man $< | $$PAGER

.PHONY: docs
docs: Doxyfile xdo.h
	doxygen

xdotool.html: xdotool.pod
	pod2html $< > $@

.PHONY: package
package: test-package-build create-package create-package-deb

.PHONY: package-deb
package-deb: test-package-build create-package-deb

.PHONY: test
test: WITH_SHELL=/bin/bash
test: xdotool libxdo.$(VERLIBSUFFIX)
	echo $(WITH_SHELL)
	if [ "$(WITH_SHELL)" = "/bin/sh" ] ; then \
		echo "Shell '$(WITH_SHELL)' fails on some Linux distros because it could"; \
		echo "be 'dash', a poorly implemented shell with bugs that break the"; \
		echo "tests. You need to use bash, zsh, or ksh to run the tests."; \
		exit 1; \
	fi
	SHELL=$(WITH_SHELL) $(MAKE) -C t

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
	rsync --exclude '.*' -a `ls -d *.pod COPYRIGHT *.c *.h examples t CHANGELIST README Makefile* version.sh platform.sh cflags.sh VERSION Doxyfile 2> /dev/null` $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -r $${NAME}
	rm VERSION

# Make sure the package we're building compiles.
.PHONY: test-package-build
test-package-build: create-package
	@NAME=xdotool-$(VERSION) && \
	echo "Testing package $$NAME" && \
	tar -zxf $${NAME}.tar.gz && \
	make -C $${NAME} && \
	make -C $${NAME} docs && \
	make -C $${NAME} test && \
	echo "Package ready: $${NAME}"; \
	rm -rf $${NAME}


### Build .deb packages for xdotool. The target 'create-package-deb' will
# create {xdotool,xdotool-doc,libxdo$(MAJOR),libxdo$(MAJOR)-dev}*.deb packages
# The reason I do this is to avoid any madness involved in dealing with
# debuild, dh_make, and related tools. '.deb' packages are an 'ar' with two
# tarballs.

DEBDIR=deb-build
create-package-deb: pre-create-package VERSION xdo_version.h
	[ -d $(DEBDIR) ] && rm -r $(DEBDIR) || true
	$(MAKE) xdotool.deb xdotool-doc.deb libxdo$(MAJOR).deb libxdo$(MAJOR)-dev.deb

%.deb: $(DEBDIR)/usr
	$(MAKE) $(DEBDIR)/$*/data.tar.gz $(DEBDIR)/$*/control.tar.gz \
	        $(DEBDIR)/$*/debian-binary
	wd=$$PWD; \
	cd $(DEBDIR)/$*; \
	  ar -qc $$wd/$*_$(VERSION)-1_$(shell uname -m).deb \
	    debian-binary control.tar.gz data.tar.gz

$(DEBDIR)/usr:
	$(MAKE) install DESTDIR=$(DEBDIR) PREFIX=/usr INSTALLMAN=/usr/share/man

$(DEBDIR)/xdotool $(DEBDIR)/xdotool-doc $(DEBDIR)/libxdo$(MAJOR) $(DEBDIR)/libxdo$(MAJOR)-dev:
	mkdir -p $@

$(DEBDIR)/%/debian-binary:
	echo "2.0" > $@

# Generate the 'control' file
$(DEBDIR)/%/control: $(DEBDIR)/%/
	sed -e 's/%VERSION%/$(VERSION)/g; s/%MAJOR%/$(MAJOR)/' \
		ext/debian/$(shell echo $* | tr -d 0-9).control > $@

# Generate the 'md5sums' file 
$(DEBDIR)/%/md5sums: $(DEBDIR)/%/ $(DEBDIR)/%/data.tar.gz 
	tar -ztf $(DEBDIR)/$*/data.tar.gz | (cd $(DEBDIR); xargs md5sum || true) > $@

# Generate the 'control.tar.gz'
$(DEBDIR)/%/control.tar.gz: $(DEBDIR)/%/control $(DEBDIR)/%/md5sums
	tar -C $(DEBDIR)/$* -zcf $(DEBDIR)/$*/control.tar.gz control md5sums 

# Build a tarball for xdotool files
$(DEBDIR)/xdotool/data.tar.gz: $(DEBDIR)/xdotool/
	tar -C $(DEBDIR) -zcf $@ usr/bin

# Build a tarball for libxdo# files
$(DEBDIR)/libxdo$(MAJOR)/data.tar.gz: $(DEBDIR)/libxdo$(MAJOR)/
	tar -C $(DEBDIR) -zcf $@ usr/lib

# Build a tarball for libxdo#-dev files
$(DEBDIR)/libxdo$(MAJOR)-dev/data.tar.gz: $(DEBDIR)/libxdo$(MAJOR)-dev/
	tar -C $(DEBDIR) -zcf $@ usr/include

# Build a tarball for xdotool-doc files
$(DEBDIR)/xdotool-doc/data.tar.gz: $(DEBDIR)/xdotool-doc/
	tar -C $(DEBDIR) -zcf $@ usr/share

