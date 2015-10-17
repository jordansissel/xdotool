#!/bin/bash
# Toggle an application - launches, focuses, or lowers an X application.  Tests in findWindow are heuristic, based on
# making it work for Geany, Chrome, Nautilus, gnome calculator gnome charmap, gnome terminal, and terminator. 
# Very useful bound to a keyboard shortcut.
# 
# Usage: 
#     toggle-app.sh WM_CLASS command args

function findWindow() {
	echo "Searching for $1" >&2
	for id in `xdotool search --class "$1"`; do
		echo "ID: $id" >&2
		TMP=`tempfile`
		xprop -id $id > $TMP
		echo "${PROP[@]}"
		test -n "$(cat $TMP | grep 'WM_STATE' | grep -v 'WM_STATE_SKIP')"; SKIP=$?
		#echo "noskip: ${SKIP}" >&2
		#if [[ $NOSKIP -eq 1 ]]; then
		#	cat $TMP | grep 'WM_STATE' >&2
		#	cat $TMP | grep 'WM_STATE'  | grep -v 'WM_STATE_SKIP' >&2
		#fi
		test -z "$(cat $TMP | grep 'WM_WINDOW_ROLE')" -o -z "$(echo ${PROP[@]} | grep 'WM_WINDOW_ROLE' | grep -v '"pop-up"')"; NOPOPUP=$?
		echo "popup: ${NOPOPUP}" >&2
		test -n "$(cat $TMP | grep 'WM_ALLOWED_ACTIONS' | grep 'WM_ACTION_MOVE')"; MOVEALLOWED=$?
		echo "immovable: ${MOVEALLOWED}" >&2
		test -n "$(cat $TMP | grep 'WM_NORMAL_HINTS')"; HASHINTS=$?
		rm $TMP
		echo "unhinted: ${HASHINTS}" >&2
		if [[ $NOPOPUP -eq 0 && $MOVEALLOWED -eq 0 && $HASHINTS -eq 0 ]]; then
			echo $id;
		else
			echo "$id does not meet requirements" >&2
		fi
	done
}

WM_CLASS=$1; shift
CMD=$1; shift
WM_ID=`findWindow $WM_CLASS | tail -1`
WM_PID=`xdotool getwindowpid $WM_ID`
ACTIVE_WM_ID=`xdotool getactivewindow`
ACTIVE_WM_PID=`xdotool getwindowpid $ACTIVE_WM_ID`
echo "Active window is ${ACTIVE_WM_ID} (${ACTIVE_WM_PID})"
if [[ -z "$WM_ID" ]]; then
	echo "No matching window id for class $WM_CLASS; running " ${CMD} "$@"
	setsid $CMD "$@" > /dev/null 2>&1 & disown $!
else
	echo "Found ID: ${WM_ID}"
	if [[ "$WM_ID" == "$ACTIVE_WM_ID" ]]; then
		echo "Lowering."
		xdotool windowlower $WM_ID
	else
		echo "Raising."
		xdotool windowactivate $WM_ID
	fi
fi
