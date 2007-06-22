CFLAGS=-g

clean:
	rm *.o

navmacro: navmacro.o xdo.o
	gcc $(CFLAGS) `pkg-config --libs x11 xtst gtk+-2.0` navmacro.o xdo.o -o $@

navmacro.o:
	gcc $(CFLAGS) `pkg-config --cflags x11 gtk+-2.0` -c navmacro.c

querytree: querytree.o
	gcc $(CFLAGS) `pkg-config --libs x11` querytree.o -o $@

querytree.o:
	gcc $(CFLAGS) `pkg-config --cflags x11` -c querytree.c

xdo:  xdo.c
	gcc -DBUILDMAIN $(CFLAGS) `pkg-config --cflags --libs x11 xtst` xdo.c -o $@

xdo.o:
	gcc $(CFLAGS) `pkg-config --cflags x11 xtst` -c xdo.c

xdotool.o:
	gcc $(CFLAGS) `pkg-config --cflags x11 xtst` -c xdotool.c

xdotool: xdotool.o xdo.o
	gcc $(CFLAGS) `pkg-config --libs x11 xtst` xdotool.o xdo.o -o $@

package:
	NAME=navmacro-`date +%Y%m%d`; \
	mkdir $${NAME}; \
	rsync --exclude .svn -av *.c *.h README Makefile examples $${NAME}/; \
	tar -zcf $${NAME}.tar.gz $${NAME}/; \
	rm -rf $${NAME}/

