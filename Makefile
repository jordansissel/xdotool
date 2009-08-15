PREFIX?=/usr/local
INSTALLBIN?=$(PREFIX)/bin
INSTALLMAN?=$(PREFIX)/man

WARNFLAGS+=-pedantic -Wall -W -Wundef \
           -Wendif-labels -Wshadow -Wpointer-arith -Wbad-function-cast \
           -Wcast-align -Wwrite-strings -Wstrict-prototypes \
           -Wmissing-prototypes -Wnested-externs -Winline \
           -Wdisabled-optimization -Wno-missing-field-initializers

CFLAGS?=-pipe -std=c99 $(WARNFLAGS)

DEFAULT_LIBS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXtst
DEFAULT_INC=-I/usr/X11R6/include -I/usr/local/include

LIBS=`pkg-config --libs x11 xtst 2> /dev/null || echo "$(DEFAULT_LIBS)"`
INC=`pkg-config --cflags x11 xtst 2> /dev/null || echo "$(DEFAULT_INC)"`

CFLAGS+=$(INC)
LDFLAGS+=$(LIBS)

all: xdotool xdotool.1

install: installprog installman

installprog: xdotool
	install -m 755 xdotool $(INSTALLBIN)/

installman: xdotool.1
	[ -d $(INSTALLMAN) ] || mkdir $(INSTALLMAN)
	[ -d $(INSTALLMAN)/man1 ] || mkdir $(INSTALLMAN)/man1
	install -m 644 xdotool.1 $(INSTALLMAN)/man1/

deinstall: uninstall
uninstall: 
	rm -f $(INSTALLBIN)/xdotool
	rm -f $(INSTALLMAN)/man1/xdotool.1

clean:
	rm -f *.o xdotool xdotool.1 || true

xdo.o: xdo.c
	$(CC) $(CFLAGS) -c xdo.c

xdotool.o: xdotool.c
	$(CC) $(CFLAGS) -c xdotool.c

xdo.c: xdo.h
xdotool.c: xdo.h

xdotool: xdotool.o xdo.o
	$(CC) $(CFLAGS) $(LDFLAGS) xdotool.o xdo.o -o $@

xdotool.1: xdotool.pod
	pod2man -c "" -r "" xdotool.pod > $@

package: test-package-build create-package

test:
	cd t/; sh run.sh

create-package: 
	@NAME=xdotool-`date +%Y%m%d`; \
	echo "Creating package: $$NAME"; \
	mkdir $${NAME}; \
	rsync --exclude .svn -a `ls -d *.pod COPYRIGHT *.c *.h examples t CHANGELIST README Makefile* 2> /dev/null` $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -rf $${NAME}/

# Make sure the package we're building compiles.
test-package-build: create-package
	@NAME=xdotool-`date +%Y%m%d`; \
	echo "Testing package $$NAME"; \
	tar -zxf $${NAME}.tar.gz; \
	make -C $${NAME} xdotool; \
	rm -rf $${NAME}/
	rm -f $${NAME}.tar.gz

