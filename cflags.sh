#!/bin/sh

system=`uname -s`
if [ "$system" = "Darwin" ] ; then
  major=`uname -r | cut -d. -f1`
  if [ "$major" -le 15 ] ; then
    # OS X 10.11 El Capitan (Darwin 15) and earlier did not have clock_gettime()
    echo "-DMISSING_CLOCK_GETTIME"
  fi
fi
