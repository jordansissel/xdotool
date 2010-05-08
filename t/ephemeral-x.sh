#!/bin/sh
# Start an ephemeral X server.
#
# This is useful for when you want to lauch an X server for a specific
# process. When that process exits, the X server will be killed.
# 

XSERVER=Xvfb
WINMGR=

prog=$0
usage() {
  echo "Usage: $prog [-x XSERVER] [-w WINDOWMANAGER] [-q] [-h] <command>"
  echo "-h   this help"
  echo "-q   quiet"
  echo "-w   window manager process to start once Xserver is up"
  echo "     (default: '$WINMGR')"
  echo "-x   Xserver (and args) to run"
  echo "     (default: '$XSERVER')"
  echo
  echo "This tool will pick an unused DISPLAY value (:0, :1, etc) and"
  echo "start an Xserver on that display, then run your command."
  echo
  echo "Examples:"
  echo "  $prog -x 'Xephyr -screen 1280x720' xterm"
  echo
  echo "  $prog -x 'Xephyr -screen 1280x720' -w "gnome-session" firefox"
}

quiet() {
  [ "0$QUIET" -eq 1 ]
}

eval "set -- $( (getopt -- x:w:qh "$@" || echo " "FAIL) | tr -d '\n')"

while [ "0$#" -gt 0 ] ; do
  case $1 in
    -x) XSERVER=$2; shift ;;
    -w) WINMGR=$2; shift ;;
    -q) QUIET=1 ;;
    -h) usage; exit ;;
    --) shift; break ;;
  esac
  shift
done

if [ "$1" = "FAIL" ] ; then
  usage
  exit 1
fi

num=-1
while true; do 
  num=$(expr $num + 1)
  xsocket=/tmp/.X11-unix/X$num
  quiet || echo "Trying :$num"
  lsof $xsocket > /dev/null 2>&1 && continue
  (
    if quiet ; then
      exec > /dev/null
      exec 2> /dev/null
    fi
    set -- $XSERVER
    cmd=$1
    shift
    exec $cmd :$num "$@"
  ) &
  xpid=$!

  healthy=0
  for i in 1 2 3 4 5 6 7 8 9 ; do
    # Break early if the xserver died
    #ps -p $xpid > /dev/null 2>&1 || break
    kill -0 $xpid || break

    # See if the xserver got a hold of the display socket.
    if lsof -p $xpid | grep -qF $xsocket ; then
      healthy=1
      break
    fi
    sleep 0.2 || sleep 1 # In case your sleep doesn't take subsecond values
  done

  if [ "0$healthy" -eq 1 ] ; then
    break
  fi
done

export DISPLAY=:$num
echo "Got display: $DISPLAY"

if [ ! -z "$WINMGR" ] ; then
  quiet || echo "Starting window manager: $WINMGR"
  $WINMGR &
  winmgrpid=$!
  sleep 1
fi

echo "$@"
(
  "$@"
)
exitcode=$?

if [ ! -z "$winmgrpid" ] ; then
  kill -9 "$winmgrpid"
fi
kill -9 "$xpid"

exit $exitcode
