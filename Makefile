CFLAGS=-Wall

DEFAULT_LIBS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lxtst
DEFAULT_INC=-I/usr/X11R6/lib -I/usr/local/lib

LIBS=`pkg-config --libs x11 xtst || echo "$(DEFAULT_LIBS)"`
INC=`pkg-config --cflags x11 xtst || echo "$(DEFAULT_INC)"`

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

package:
	NAME=xdotool-`date +%Y%m%d`; \
	mkdir $${NAME}; \
	rsync --exclude .svn -av *.c *.h README Makefile* $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -rf $${NAME}/

