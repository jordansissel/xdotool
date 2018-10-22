#!/bin/sh

VERSION=$1
LIBDIR=$2
INCLUDEDIR=$3

cat <<ENDPC
libdir=${LIBDIR}
includedir=${INCLUDEDIR}

Name: libxdo
Description: fake keyboard/mouse input, window management, and more
Version: ${VERSION}
Requires: x11
Libs: -L\${libdir} -lxdo
Cflags: -I\${includedir}
ENDPC
