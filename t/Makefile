
all-headless: test-xvfb test-xvfb-openbox test-xvfb-gnome
all-xephyr: test-xephyr test-xephyr-openbox test-xephyr-gnome
all: all-headless all-xephyr 

test: all

do-test:
	@echo " => Running tests on $${XSERVER%% *}/$${WM:-no-windowmanager}"; \
	set -e; \
	for i in test_search.rb; do \
		sh ephemeral-x.sh -q -x "$$XSERVER" -w "$$WM" ./run.sh ruby $$i; \
	done 

test-xephyr:
	QUIET=1 XSERVER="Xephyr -ac -screen 1280x768x24" $(MAKE) do-test

test-xvfb:
	QUIET=0 XSERVER="Xvfb -ac -screen 0 1280x768x24" $(MAKE) do-test

test-xvnc:
	QUIET=0 XSERVER="Xvnc" $(MAKE) do-test

test-xvfb-openbox:
	WM=openbox-session $(MAKE) test-xvfb

test-xephyr-openbox:
	WM=openbox-session $(MAKE) test-xephyr

test-xvfb-gnome:
	WM=gnome-session $(MAKE) test-xvfb

test-xephyr-gnome:
	WM=gnome-session $(MAKE) test-xephyr
