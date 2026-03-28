# `xdotool` - x11 automation tool

`xdotool` lets you simulate keyboard input and mouse activity, move and resize windows, etc. It does this using X11’s XTEST extension and other Xlib functions.

⚠ Note: If you are using Wayland, please be aware this software will not work correctly. See the 'Wayland' section below for more detail.

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
* macOS: `brew install xdotool` or `sudo port install xdotool`
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

## Wayland

Wayland is a very different graphics system than X11 (which you might know as Xorg or X). Wayland has some X11 compatibility, but for the purposes of xdotool, many things do not work correctly. Typing, window searching, and many other functions of xdotool do not work, and it is unclear if they could ever work. That does not mean these functions can't be achieved with Wayland, it just means that you usually need to go through a Wayland compositor.

Some other tools that might help you if you use Wayland are:

* [ydotool](https://github.com/ReimuNotMoe/ydotool) - a tool for sending mouse and keyboard events using Linux's uinput sytem
* [dotool](https://git.sr.ht/~geb/dotool) - a tool for sending mouse and keyboard events using Linux's uinput sytem

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


