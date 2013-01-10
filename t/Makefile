DEBUG?=0
ifneq ($(DEBUG), 0)
RUBYFLAGS=-d
endif

HEADLESS_TESTS?=test-xvfb-nowm test-xvfb-openbox test-xvfb-gnome
XEPHYR_TESTS?=test-xephyr-nowm test-xephyr-openbox test-xephyr-gnome

all-headless: $(HEADLESS_TESTS)
all-xephyr: $(XEPHYR_TESTS)
all: all-headless all-xephyr 

test: all

# The loop-* targets are mainly for spinning until a test fails 
# so we can look for flakey tests.
loop-headless:
	while $(MAKE) $(MAKEARGS) all-headless; do true; done

loop-xephyr:
	while $(MAKE) $(MAKEARGS) all-xephyr; do true; done

loop-all:
	while $(MAKE) $(MAKEARGS) all; do true; done

do-test:
	@echo " => Running tests on $${XSERVER%% *}/$${WM:-no-windowmanager}"; \
	set -e; \
	make -C ../; \
	sh check-for-tools.sh || exit 0; \
	for i in test_*.rb ; do \
	  echo "Running $$i"; \
	  sh ephemeral-x.sh -q -x "$$XSERVER" -w "$$WM" ./run.sh ruby $(RUBYFLAGS) $$i $(TESTFLAGS); \
	done 

#sh ephemeral-x.sh -q -x "$$XSERVER" -w "$$WM" ./run.sh -- ruby alltests.rb

test-xephyr:
	QUIET=1 XSERVER="Xephyr -ac -screen 1280x768x24" $(MAKE) do-test

test-xvfb:
	QUIET=0 XSERVER="Xvfb -ac -screen 0 1280x768x24" $(MAKE) do-test

test-xvnc:
	QUIET=0 XSERVER="Xvnc" $(MAKE) do-test

test-xvfb-nowm:
	WM=none $(MAKE) test-xvfb

test-xephyr-nowm:
	WM=none $(MAKE) test-xephyr

test-xvfb-openbox:
	WM=openbox-session $(MAKE) test-xvfb

test-xephyr-openbox:
	WM=openbox-session $(MAKE) test-xephyr

test-xvfb-gnome:
	@echo "=> Note about gnome: Gnome tests will fail if you are already running gnome."
	WM=gnome-session $(MAKE) test-xvfb

test-xephyr-gnome:
	@echo "=> Note about gnome: Gnome tests will fail if you are already running gnome."
	WM=gnome-session $(MAKE) test-xephyr
