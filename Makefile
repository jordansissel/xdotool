CFLAGS+=-Wall

DEFAULT_LIBS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXtst
DEFAULT_INC=-I/usr/X11R6/include -I/usr/local/include

LIBS=`pkg-config --libs x11 xtst 2> /dev/null || echo "$(DEFAULT_LIBS)"`
INC=`pkg-config --cflags x11 xtst 2> /dev/null || echo "$(DEFAULT_INC)"`

CFLAGS+=$(INC)
LDFLAGS+=$(LIBS)

all: xdotool

clean:
	rm -f *.o || true

xdo.o: xdo.c
	gcc $(CFLAGS) -c xdo.c

xdotool.o: xdotool.c
	gcc $(CFLAGS) -c xdotool.c

xdo.c: xdo.h
xdotool.c: xdo.h

xdotool: xdotool.o xdo.o
	gcc $(CFLAGS) $(LDFLAGS) xdotool.o xdo.o -o $@

package: test-package-build create-package

create-package:
	@NAME=fex-`date +%Y%m%d`; \
	echo "Creating package: $$NAME"; \
	mkdir $${NAME}; \
	rsync --exclude .svn -a `ls *.c *.h CHANGELIST README Makefile* 2> /dev/null` $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -rf $${NAME}/

# Make sure the package we're building compiles.
test-package-build: create-package
	@NAME=fex-`date +%Y%m%d`; \
	echo "Testing package $$NAME"; \
	tar -zxf $${NAME}.tar.gz; \
	make -C $${NAME} fex; \
	rm -rf $${NAME}/
	rm -f $${NAME}.tar.gz

