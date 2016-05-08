# FAQ

## Can `xdotool` work with hexadecimal window IDs?

Yes. You can easily check this, by using `wmctrl` as
an alternative way to obtain a window ID in hexadecimal.

This will show the window ID in hexadecimal of
a window with title `My dialog title`:

```
wmctrl -l | egrep "My dialog title" | cut -f 1 -d ' '
```

I got the value of `0x02800003` displayed on my screen, 
but you will probably have a different value.

Feeding that hexadecimal value to `xdotool` does activate the window:

```
xdotool windowactivate $(wmctrl -l | egrep "My dialog title" | cut -f 1 -d ' ')
```

## How to let `xdotool` work with Qt applications?

`xdotool` can unsually extract the ID of a Qt application 
from its window title, as shown by 
[this StackOverflow question](http://stackoverflow.com/questions/37050159/xdotool-cannot-find-qt-application-window-where-wmctrl-can).

If something is wrong, a workaround is to use `wmctrl`
to obtain the window ID:

```
xdotool windowactivate $(wmctrl -l | egrep "My dialog title" | cut -f 1 -d ' ')
```

Replace 'My dialog title' for the window name you are looking for.

Note that `wmctrl` displays the window ID in hexadecimal. This is no
problem for `xdotool`!