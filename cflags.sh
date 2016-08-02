#!/bin/sh

osxver=`sw_vers -productVersion 2> /dev/null`
if [ $? -eq 0 ] ; then
  minor=`echo "$osxver" | cut -d. -f2`
  if [ "$minor" -le 11 ] ; then
    # Versions of OSX before 10.12 (aka "macOS Sierra") did not have clock_gettime()
    echo "-DMISSING_CLOCK_GETTIME"
  fi
fi
