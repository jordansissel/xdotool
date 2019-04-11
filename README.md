# xdotool
Simulate input from the mouse and keyboard very easily.
Also supports window manager actions such as moving, activating, and other actions on windows.
## libxdo
C library for doing the same.

## Documentation
See the [website](http://www.semicomplete.com/projects/xdotool/) or manpage (see below) for more up-to-date documentation.

## Installation
xdotool is included in many package managers

|Operating System|command|
|------|-------|
|Arch Linux |`pacman -S xdotool`|
|Debian/Ubuntu|`apt-get install xdotool`|
|FreeBSD|`pkg install xdotool`|
|Fedora|`dnf install xdotool`|
|macOS|`brew install xdotool`|
|OpenSUSE|`zypper install xdotool`|

## Compilation

Compile: `make`

Run tests: `make test` (requires ruby, Xvfb)

Install: `make install`

Remove: `make uninstall`

You may have to set `PREFIX` to the location you want to install to.
The default `PREFIX` is `/usr/local`

For packagers, there's also support for DESTDIR for staged install.

Also, see the manpage, which you can generate by running `make showman`
Note: the manpage will be installed during `make install`

## Questions
See the [FAQ](FAQ.md)