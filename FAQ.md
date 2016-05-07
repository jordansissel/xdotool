# FAQ

## Can `xdotool` work with hexadecimal window IDs?

Yes, see 'How to let `xdotool` work with Qt applications?'.

## How to let `xdotool` work with Qt applications?

`xodotool` cannot directly extract the ID of a Qt application 
from its window title (yet). A workaround is to use `wmctrl`
to obtain the window ID:

```
xdotool windowactivate $(wmctrl -l | egrep "My dialog title" | cut -f 1 -d ' ')
```

Replace 'My dialog title' for the window name you are looking for.

Note that `wmctrl` displays the window ID in hexadecimal. This is no
problem for `xdotool`!