#!/bin/sh

. ./test.rc

xdotool="../xdotool"
$xdotool search --maxdepth 0 . | wc -l | try grep '^0$'
