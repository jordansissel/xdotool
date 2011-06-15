#!/bin/sh

uname=$(uname)

libsuffix() {
  case $uname in
    Darwin)
      if [ -z "$1" ] ; then
        echo "dylib"
      else
        echo "$1.dylib"
      fi
      ;;
    *)
      if [ -z "$1" ] ; then
        echo "so"
      else
        echo so.$1
      fi
      ;;
  esac
}

dynlibflag() {
  case $uname in
    Darwin) echo "-dynamiclib" ;;
    *) echo "-shared" ;;
  esac
}

libnameflag() {
  MAJOR=$1
  INSTALLLIB=$2
  case $uname in
    Darwin) echo "-Wl,-install_name,$INSTALLLIB/libxdo.$MAJOR.dylib" ;;
    *) echo "-Wl,-soname=libxdo.so.$MAJOR" ;;
  esac
}

extralibs() {
  case $uname in
    Linux|GNU/kFreeBSD|GNU) echo "-lrt" ;;
  esac
}

command=$1
shift
case $command in
  libsuffix) $command "$@" ;;
  dynlibflag) $command "$@" ;;
  libnameflag) $command "$@" ;;
  extralibs) $command "$@" ;;
  *) 
    echo "Invalid $0 command, '$command'" >&2
    exit 1
    ;;
esac

