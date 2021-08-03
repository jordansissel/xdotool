# `xdotool` - x11 automation tool

`xdotool` lets you simulate keyboard input and mouse activity, move and resize windows, etc. It does this using X11’s XTEST extension and other Xlib functions.

⚠ Note: If you are using Wayland, please be aware this software will not work correctly. ⚠

With xdotool, you can search for windows and move, resize, hide, and modify
window properties like the title. If your window manager supports it, you can
use xdotool to switch desktops, move windows between desktops, and change the
number of desktops.

Also in this repository is `libxdo`, a C library for doing the same.

You may view the user documentation in [`xdotool.pod`](https://github.com/jordansissel/xdotool/blob/master/xdotool.pod)

## Installation

You may find xdotool in your distribution packaging:

* Debian and Ubuntu: `apt-get install xdotool`
* Fedora: `dnf install xdotool`
* FreeBSD: `pkg install xdotool`
* OSX: `brew install xdotool`
* OpenSUSE: `zypper install xdotool`

## Basic Usage

### Typing

From your terminal, run:

```
xdotool type "Hello world"
```

### Sending keys

```
xdotool key ctrl+l
```

The above will simulate the keystrokes as if you pressed the control key, then the "L" key, and then released both. This is useful for simulating hotkeys.

### Closing a window

```
xdotool selectwindow windowclose
```

This will close the first window you click on.

## Cool Tricks

### Bring up Firefox and focus the URL bar

```
xdotool search "Mozilla Firefox" windowactivate --sync key --clearmodifiers ctrl+l
```

### Resize all visible gnome-terminal windows

```
xdotool search --onlyvisible --classname "gnome-terminal" windowsize %@ 500
500
```

## Building / Compiling

Prerequisites:
* X11 libraries: xlib, xtst, xi, xkbcommon, xinerama

How to compile and install:

* Compile: make
* Install: make install
* Remove: make uninstall

You may have to set 'PREFIX' to the location you want to install to. 
The default PREFIX is /usr/local

For packagers, there's also support for DESTDIR for staged install.

Also, see the manpage, which you can generate by running:

```
  make showman
```

Note: the manpage will be installed during 'make install'


