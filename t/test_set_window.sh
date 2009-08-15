#!/bin/sh

. ./test.rc

xdotool="../xdotool"
mkwindow

sleep 1

wid=`$xdotool search xdotool_test_window`
try $xdotool set_window --name "NAME" $wid
try $xdotool set_window --icon "ICON" $wid
try $xdotool set_window --class "CLASS" $wid
try $xdotool set_window --classname "CLASSNAME" $wid
try $xdotool set_window --role "ROLE" $wid

prop="$(xprop -id $wid)"

echo "$prop" | try grep -F 'WM_CLASS(STRING) = "CLASSNAME", "CLASS"'
echo "$prop" | try grep -F 'WM_NAME(STRING) = "NAME"'
echo "$prop" | try grep -F 'WM_ICON_NAME(STRING) = "ICON"'
echo "$prop" | try grep -F 'WM_WINDOW_ROLE(STRING) = "ROLE"'

kill $windowpid
