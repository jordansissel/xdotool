CFLAGS=-Wall

all: xdotool

clean:
	rm -f *.o || true

xdo.o: xdo.c
	gcc $(CFLAGS) `pkg-config --cflags x11 xtst` -c xdo.c

xdotool.o: xdotool.c
	gcc $(CFLAGS) `pkg-config --cflags x11 xtst` -c xdotool.c

xdo.c: xdo.h
xdotool.c: xdo.h

xdotool: xdotool.o xdo.o
	gcc $(CFLAGS) `pkg-config --libs x11 xtst` xdotool.o xdo.o -o $@

package:
	NAME=xdotool-`date +%Y%m%d`; \
	mkdir $${NAME}; \
	rsync --exclude .svn -av *.c *.h README Makefile* $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -rf $${NAME}/

