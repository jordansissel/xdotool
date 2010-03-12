
test:
	./run.sh

all-headless: test-xvfb test-xvfb-openbox test-xvfb-gnome
all: all-headless test-xephyr test-xephyr-openbox test-xephyr-gnome

test-xephyr:
	QUIET=1 XSERVER="Xephyr -ac -screen 1280x768x24" TEST_DISPLAY=:5 $(MAKE) test

test-xvfb:
	QUIET=1 XSERVER="Xvfb -ac -screen 0 1280x768x24" TEST_DISPLAY=:5 $(MAKE) test

test-xvfb-openbox:
	WM=openbox-session $(MAKE) test-xvfb

test-xephyr-openbox:
	WM=openbox-session $(MAKE) test-xephyr

test-xvfb-gnome:
	WM=gnome-session $(MAKE) test-xvfb

test-xephyr-gnome:
	WM=gnome-session $(MAKE) test-xephyr
