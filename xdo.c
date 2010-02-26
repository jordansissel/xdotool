/* xdo library
 *
 * - getwindowfocus contributed by Lee Pumphret
 * - keysequence_{up,down} contributed by Magnus Boman
 *
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif /* _XOPEN_SOURCE */

#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <regex.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include "xdo.h"
#include "xdo_util.h"
#include "xdo_version.h"

static void _xdo_populate_charcode_map(xdo_t *xdo);
static int _xdo_has_xtest(const xdo_t *xdo);

static int _xdo_keycode_from_char(const xdo_t *xdo, char key);
static int _xdo_get_shiftcode_if_needed(const xdo_t *xdo, char key);

static int _xdo_keysequence_to_keycode_list(const xdo_t *xdo, const char *keyseq,
                                            charcodemap_t **keys, int *nkeys);
static int _xdo_keysequence_do(const xdo_t *xdo, Window window, const char *keyseq,
                               int pressed, int *modifier);
static int _xdo_ewmh_is_supported(const xdo_t *xdo, const char *feature);
static void _xdo_init_xkeyevent(const xdo_t *xdo, XKeyEvent *xk);
void _xdo_send_key(const xdo_t *xdo, Window window, int keycode, int modstate,
                   int is_press, useconds_t delay);

static int _xdo_query_keycode_to_modifier(const xdo_t *xdo, KeyCode keycode);
static int _xdo_cached_keycode_to_modifier(const xdo_t *xdo, KeyCode keycode);
static int _xdo_cached_modifier_to_keycode(const xdo_t *xdo, int modmask);

static int _is_success(const char *funcname, int code);

/* context-free functions */
static char _keysym_to_char(const char *keysym);

xdo_t* xdo_new(char *display_name) {
  Display *xdpy;

  if ((xdpy = XOpenDisplay(display_name)) == NULL) {
    fprintf(stderr, "Error: Can't open display: %s\n", display_name);
    return NULL;
  }

  return xdo_new_with_opened_display(xdpy, display_name, 1);
}

xdo_t* xdo_new_with_opened_display(Display *xdpy, const char *display,
                                   int close_display_when_freed) {
  xdo_t *xdo = NULL;

  if (xdpy == NULL) {
    fprintf(stderr, "xdo_new: xdisplay I was given is a null pointer\n");
    return NULL;
  }

  /* XXX: Check for NULL here */
  xdo = malloc(sizeof(xdo_t));
  memset(xdo, 0, sizeof(xdo_t));

  xdo->xdpy = xdpy;
  xdo->close_display_when_freed = close_display_when_freed;

  if (display == NULL)
    display = "unknown";

  if (!_xdo_has_xtest(xdo)) {
    fprintf(stderr, "Error: XTEST extension unavailable on '%s'.", 
            xdo->display_name);
    xdo_free(xdo);
    return NULL;
  }

  xdo->modmap = XGetModifierMapping(xdo->xdpy);
  _xdo_populate_charcode_map(xdo);
  return xdo;
}

void xdo_free(xdo_t *xdo) {
  if (xdo->display_name)
    free(xdo->display_name);
  if (xdo->charcodes)
    free(xdo->charcodes);
  if (xdo->xdpy && xdo->close_display_when_freed)
    XCloseDisplay(xdo->xdpy);
  if (xdo->modmap)
    XFreeModifiermap(xdo->modmap);

  free(xdo);
}

const char *xdo_version(void) {
  return XDO_VERSION;
}

int xdo_window_map(const xdo_t *xdo, Window wid) {
  int ret = 0;
  ret = XMapWindow(xdo->xdpy, wid);
  XFlush(xdo->xdpy);
  return _is_success("XMapWindow", ret == 0);
}

int xdo_window_unmap(const xdo_t *xdo, Window wid) {
  int ret = 0;
  ret = XUnmapWindow(xdo->xdpy, wid);
  XFlush(xdo->xdpy);
  return _is_success("XUnmapWindow", ret == 0);
}

int xdo_window_move(const xdo_t *xdo, Window wid, int x, int y) {
  XWindowChanges wc;
  int ret = 0;
  wc.x = x;
  wc.y = y;

  ret = XConfigureWindow(xdo->xdpy, wid, CWX | CWY, &wc);
  return _is_success("XConfigureWindow", ret == 0);
}

int xdo_window_setsize(const xdo_t *xdo, Window wid, int width, int height, int flags) {
  XWindowChanges wc;
  int ret = 0;
  int cw_flags = 0;

  wc.width = width;
  wc.height = height;

  if (flags & SIZE_USEHINTS) {
    XSizeHints hints;
    long supplied_return;
    memset(&hints, 0, sizeof(hints));
    XGetWMNormalHints(xdo->xdpy, wid, &hints, &supplied_return);
    if (supplied_return & PResizeInc) {
      wc.width *= hints.width_inc;
      wc.height *= hints.height_inc;
    } else {
      fprintf(stderr, "No size hints found for this window\n");
    }

    if (supplied_return & PBaseSize) {
      wc.width += hints.base_width;
      wc.height += hints.base_height;
    }

  }

  if (width > 0)
    cw_flags |= CWWidth;
  if (height > 0)
    cw_flags |= CWHeight;

  ret = XConfigureWindow(xdo->xdpy, wid, cw_flags, &wc);
  XFlush(xdo->xdpy);
  return _is_success("XConfigureWindow", ret == 0);
}

int xdo_window_setclass (const xdo_t *xdo, Window wid, const char *name, const char *class) {
  int ret = 0;
  XClassHint *hint = XAllocClassHint();
  
  ret = XGetClassHint(xdo->xdpy, wid, hint);
  if (ret == 0) {
    return _is_success("XGetClassHint", ret == 0);
  }

  if (name != NULL)
    hint->res_name = (char*)name;

  if(class != NULL)
    hint->res_class = (char*)class;

  ret = XSetClassHint(xdo->xdpy, wid, hint);
  XFree(hint);
  return _is_success("XSetClassHint", ret == 0);
}

int xdo_window_setprop (const xdo_t *xdo, Window wid, const char *property, const char *value) {
  
  char netwm_property[256] = "_NET_";
  int ret = 0;
  strncat(netwm_property, property, strlen(property));

  // Change the property
  ret = XChangeProperty(xdo->xdpy, wid, 
                        XInternAtom(xdo->xdpy, property, False), 
                        XInternAtom(xdo->xdpy, "STRING", False), 8, 
                        PropModeReplace, (unsigned char*)value, strlen(value));
  if (ret == 0) {
    return _is_success("XChangeProperty", ret == 0);
  }

  // Change _NET_<property> just in case for simpler NETWM compliance?
  ret = XChangeProperty(xdo->xdpy, wid, 
                        XInternAtom(xdo->xdpy, netwm_property, False), 
                        XInternAtom(xdo->xdpy, "STRING", False), 8, 
                        PropModeReplace, (unsigned char*)value, strlen(value));
  return _is_success("XChangeProperty", ret == 0);
}

int xdo_window_focus(const xdo_t *xdo, Window wid) {
  int ret = 0;
  ret = XSetInputFocus(xdo->xdpy, wid, RevertToParent, CurrentTime);
  XFlush(xdo->xdpy);
  return _is_success("XSetInputFocus", ret == 0);
}

int xdo_window_activate(const xdo_t *xdo, Window wid) {
  int ret = 0;
  long desktop = 0;
  XEvent xev;
  XWindowAttributes wattr;

  if (_xdo_ewmh_is_supported(xdo, "_NET_ACTIVE_WINDOW") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_ACTIVE_WINDOW, "
            "so the attempt to activate the window was aborted.\n");
    return XDO_ERROR;
  }

  /* If this window is on another desktop, let's go to that desktop first */

  if (_xdo_ewmh_is_supported(xdo, "_NET_WM_DESKTOP") == True
      && _xdo_ewmh_is_supported(xdo, "_NET_CURRENT_DESKTOP") == True) {
    xdo_get_desktop_for_window(xdo, wid, &desktop);
    xdo_set_current_desktop(xdo, desktop);
  }

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.display = xdo->xdpy;
  xev.xclient.window = wid;
  xev.xclient.message_type = XInternAtom(xdo->xdpy, "_NET_ACTIVE_WINDOW", False);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 2L; /* 2 == Message from a window pager */
  xev.xclient.data.l[1] = CurrentTime;

  XGetWindowAttributes(xdo->xdpy, wid, &wattr);
  ret = XSendEvent(xdo->xdpy, wattr.screen->root, False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &xev);

  /* XXX: XSendEvent returns 0 on conversion failure, nonzero otherwise.
   * Manpage says it will only generate BadWindow or BadValue errors */
  return _is_success("XSendEvent[EWMH:_NET_ACTIVE_WINDOW]", ret == 0);
}

int xdo_set_number_of_desktops(const xdo_t *xdo, long ndesktops) {
  /* XXX: This should support passing a screen number */
  XEvent xev;
  Window root;
  int ret = 0;

  if (_xdo_ewmh_is_supported(xdo, "_NET_NUMBER_OF_DESKTOPS") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_NUMBER_OF_DESKTOPS, "
            "so the attempt to change the number of desktops was aborted.\n");
    return XDO_ERROR;
  }

  root = RootWindow(xdo->xdpy, 0);

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.display = xdo->xdpy;
  xev.xclient.window = root;
  xev.xclient.message_type = XInternAtom(xdo->xdpy, "_NET_NUMBER_OF_DESKTOPS", 
                                         False);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = ndesktops;

  ret = XSendEvent(xdo->xdpy, root, False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &xev);

  return _is_success("XSendEvent[EWMH:_NET_NUMBER_OF_DESKTOPS]", ret == 0);
}

int xdo_get_number_of_desktops(const xdo_t *xdo, long *ndesktops) {
  Atom type;
  int size;
  long nitems;
  unsigned char *data;
  Window root;
  Atom request;

  if (_xdo_ewmh_is_supported(xdo, "_NET_NUMBER_OF_DESKTOPS") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_NUMBER_OF_DESKTOPS, "
            "so the attempt to query the number of desktops was aborted.\n");
    return XDO_ERROR;
  }

  request = XInternAtom(xdo->xdpy, "_NET_NUMBER_OF_DESKTOPS", False);
  root = XDefaultRootWindow(xdo->xdpy);

  data = xdo_getwinprop(xdo, root, request, &nitems, &type, &size);

  if (nitems > 0) {
    *ndesktops = *((long*)data);
  } else {
    *ndesktops = 0;
  }
  free(data);

  return _is_success("XGetWindowProperty[_NET_NUMBER_OF_DESKTOPS]",
                     *ndesktops == 0);
}

int xdo_set_current_desktop(const xdo_t *xdo, long desktop) {
  /* XXX: This should support passing a screen number */
  XEvent xev;
  Window root;
  int ret = 0;

  root = RootWindow(xdo->xdpy, 0);

  if (_xdo_ewmh_is_supported(xdo, "_NET_CURRENT_DESKTOP") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_CURRENT_DESKTOP, "
            "so the attempt to change desktops was aborted.\n");
    return XDO_ERROR;
  }

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.display = xdo->xdpy;
  xev.xclient.window = root;
  xev.xclient.message_type = XInternAtom(xdo->xdpy, "_NET_CURRENT_DESKTOP", 
                                         False);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = desktop;
  xev.xclient.data.l[1] = CurrentTime;

  ret = XSendEvent(xdo->xdpy, root, False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &xev);

  return _is_success("XSendEvent[EWMH:_NET_CURRENT_DESKTOP]", ret == 0);
}

int xdo_get_current_desktop(const xdo_t *xdo, long *desktop) {
  Atom type;
  int size;
  long nitems;
  unsigned char *data;
  Window root;

  Atom request;

  if (_xdo_ewmh_is_supported(xdo, "_NET_CURRENT_DESKTOP") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_CURRENT_DESKTOP, "
            "so the query for the current desktop was aborted.\n");
    return XDO_ERROR;
  }

  request = XInternAtom(xdo->xdpy, "_NET_CURRENT_DESKTOP", False);
  root = XDefaultRootWindow(xdo->xdpy);

  data = xdo_getwinprop(xdo, root, request, &nitems, &type, &size);

  if (nitems > 0) {
    *desktop = *((long*)data);
  } else {
    *desktop = -1;
  }
  free(data);

  return _is_success("XGetWindowProperty[_NET_CURRENT_DESKTOP]",
                     *desktop == -1);
}

int xdo_set_desktop_for_window(const xdo_t *xdo, Window wid, long desktop) {
  XEvent xev;
  int ret = 0;
  XWindowAttributes wattr;
  XGetWindowAttributes(xdo->xdpy, wid, &wattr);

  if (_xdo_ewmh_is_supported(xdo, "_NET_WM_DESKTOP") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_WM_DESKTOP, "
            "so the attempt to change a window's desktop location was "
            "aborted.\n");
    return XDO_ERROR;
  }

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.display = xdo->xdpy;
  xev.xclient.window = wid;
  xev.xclient.message_type = XInternAtom(xdo->xdpy, "_NET_WM_DESKTOP", 
                                         False);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = desktop;
  xev.xclient.data.l[1] = 2; /* indicate we are messaging from a pager */

  ret = XSendEvent(xdo->xdpy, wattr.screen->root, False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &xev);

  return _is_success("XSendEvent[EWMH:_NET_WM_DESKTOP]", ret == 0);
}

int xdo_get_desktop_for_window(const xdo_t *xdo, Window wid, long *desktop) {
  Atom type;
  int size;
  long nitems;
  unsigned char *data;
  Atom request;

  if (_xdo_ewmh_is_supported(xdo, "_NET_WM_DESKTOP") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_WM_DESKTOP, "
            "so the attempt to query a window's desktop location was "
            "aborted.\n");
    return XDO_ERROR;
  }

  request = XInternAtom(xdo->xdpy, "_NET_WM_DESKTOP", False);

  data = xdo_getwinprop(xdo, wid, request, &nitems, &type, &size);

  if (nitems > 0) {
    *desktop = *((long*)data);
  } else {
    *desktop = -1;
  }
  free(data);

  return _is_success("XGetWindowProperty[_NET_WM_DESKTOP]",
                     *desktop == -1);
}

int xdo_window_get_active(const xdo_t *xdo, Window *window_ret) {
  Atom type;
  int size;
  long nitems;
  unsigned char *data;
  Atom request;
  Window root;

  if (_xdo_ewmh_is_supported(xdo, "_NET_ACTIVE_WINDOW") == False) {
    fprintf(stderr,
            "Your windowmanager claims not to support _NET_ACTIVE_WINDOW, "
            "so the attempt to query the active window aborted.\n");
    return XDO_ERROR;
  }

  request = XInternAtom(xdo->xdpy, "_NET_ACTIVE_WINDOW", False);
  root = XDefaultRootWindow(xdo->xdpy);
  data = xdo_getwinprop(xdo, root, request, &nitems, &type, &size);

  if (nitems > 0) {
    *window_ret = *((Window*)data);
  } else {
    *window_ret = 0;
  }
  free(data);

  return _is_success("XGetWindowProperty[_NET_ACTIVE_WINDOW]",
                     *window_ret == 0);
}

/* XRaiseWindow is ignored in ion3 and Gnome2. Is it even useful? */
int xdo_window_raise(const xdo_t *xdo, Window wid) {
  int ret = 0;
  ret = XRaiseWindow(xdo->xdpy, wid);
  XFlush(xdo->xdpy);
  return _is_success("XRaiseWindow", ret == 0);
}

/* XXX: Include 'screen number' support? */
int xdo_mousemove(const xdo_t *xdo, int x, int y)  {
  int ret = 0;
  ret = XTestFakeMotionEvent(xdo->xdpy, -1, x, y, CurrentTime);
  XFlush(xdo->xdpy);
  return _is_success("XTestFakeMotionEvent", ret == 0);
}

int xdo_mousemove_relative(const xdo_t *xdo, int x, int y)  {
  int ret = 0;
  ret = XTestFakeRelativeMotionEvent(xdo->xdpy, x, y, CurrentTime);
  XFlush(xdo->xdpy);
  return _is_success("XTestFakeRelativeMotionEvent", ret == 0);
}

int xdo_mousedown(const xdo_t *xdo, Window window, int button) {
  int ret = 0;

  if (window == 0) {
    ret = XTestFakeButtonEvent(xdo->xdpy, button, True, CurrentTime);
  } else {
    /* Send to specific window */
    fprintf(stderr, "Not implemented\n");
    ret = XDO_ERROR;
  }

  XFlush(xdo->xdpy);
  return _is_success("XTestFakeButtonEvent(down)", ret == 0);
}

int xdo_mouseup(const xdo_t *xdo, Window window, int button) {
  int ret = 0;

  if (window == 0) {
    ret = XTestFakeButtonEvent(xdo->xdpy, button, False, CurrentTime);
  } else {
    /* Send to specific window */
    fprintf(stderr, "Not implemented\n");
    ret = XDO_ERROR;
  }
  XFlush(xdo->xdpy);
  return _is_success("XTestFakeButtonEvent(up)", ret == 0);
}

int xdo_mouselocation(const xdo_t *xdo, int *x_ret, int *y_ret, int *screen_num_ret) {
  int ret = False;
  int x = 0, y = 0, screen_num = 0;
  int i = 0;
  Window dummy_win = 0;
  int dummy_int = 0;
  unsigned int dummy_uint = 0;
  int screencount = ScreenCount(xdo->xdpy);

  for (i = 0; i < screencount; i++) {
    Screen *screen = ScreenOfDisplay(xdo->xdpy, i);
    ret = XQueryPointer(xdo->xdpy, RootWindowOfScreen(screen),
                        &dummy_win, &dummy_win,
                        &x, &y, &dummy_int, &dummy_int, &dummy_uint);
    if (ret == True) {
      screen_num = i;
      break;
    }
  }

  if (ret == True) {
    if (x_ret != NULL) *x_ret = x;
    if (y_ret != NULL) *y_ret = y;
    if (screen_num_ret != NULL) *screen_num_ret = screen_num;
  }

  return _is_success("XQueryPointer", ret == False);
}

int xdo_click(const xdo_t *xdo, Window window, int button) {
  int ret = 0;
  ret = xdo_mousedown(xdo, window, button);
  if (ret != XDO_SUCCESS)
    return ret;
  ret = xdo_mouseup(xdo, window, button);
  return ret;
}

/* XXX: Return proper code if errors found */
int xdo_type(const xdo_t *xdo, Window window, char *string, useconds_t delay) {
  int i = 0;
  char key = '\0';
  int keycode = 0;
  int shiftcode = 0;
  int modstate = 0;

  /* Since we're doing down/up, the delay should be based on the number
   * of keys pressed (including shift). Since up/down is two calls,
   * divide by two. */
  delay /= 2;

  /* XXX: Add error handling */
  for (i = 0; string[i] != '\0'; i++) {
    modstate = 0; /* No modifiers by default */
    key = string[i];
    keycode = _xdo_keycode_from_char(xdo, key);
    shiftcode = _xdo_get_shiftcode_if_needed(xdo, key);
    if (shiftcode) {
      modstate |= ShiftMask;
    }

    _xdo_send_key(xdo, window, keycode, modstate, True, delay);
    _xdo_send_key(xdo, window, keycode, modstate, False, delay);

    /* XXX: Flush here or at the end? or never? */
    XFlush(xdo->xdpy);
  }

  return XDO_SUCCESS;
}

int _xdo_keysequence_do(const xdo_t *xdo, Window window, const char *keyseq, int pressed, int *modifier) {
  int ret = 0;
  charcodemap_t *keys = NULL;
  int nkeys = 0;

  if (_xdo_keysequence_to_keycode_list(xdo, keyseq, &keys, &nkeys) == False) {
    fprintf(stderr, "Failure converting key sequence '%s' to keycodes\n", keyseq);
    return 1;
  }

  ret = xdo_keysequence_list_do(xdo, window, keys, nkeys, pressed, modifier);
  if (keys != NULL) {
    free(keys);
  }

  return ret;
}

int xdo_keysequence_list_do(const xdo_t *xdo, Window window, charcodemap_t *keys, 
                            int nkeys, int pressed, int *modifier) {
  int i = 0;
  int modstate = 0;
  int keymapchanged;

  /* Find an unused keycode in case we need to bind unmapped keysyms */
  KeySym *keysyms = NULL;
  int keysyms_per_keycode = 0;
  int scratch_keycode = 0; /* Scratch space for temporary keycode bindings */
  keysyms = XGetKeyboardMapping(xdo->xdpy, xdo->keycode_low, 
                                xdo->keycode_high - xdo->keycode_low,
                                &keysyms_per_keycode);
  for (i = xdo->keycode_low; i <= xdo->keycode_high; i++) {
    int j = 0;
    int key_is_empty = 1;
    for (j = 0; j < keysyms_per_keycode; j++) {
      char *symname;
      int symindex = (i - xdo->keycode_low) * keysyms_per_keycode + j;
      symname = XKeysymToString(keysyms[symindex]);
      if (keysyms[symindex] != 0) {
        key_is_empty = 0;
      }
    }
    if (key_is_empty) {
      scratch_keycode = i;
      break;
    }
  }
  XFree(keysyms);

  /* Allow passing NULL for modifier in case we don't care about knowing
   * the modifier map state after we finish */
  if (modifier == NULL)
    modifier = &modstate;

  for (i = 0; i < nkeys; i++) {
    if (keys[i].needs_binding == 1) {
      KeySym keysym_list[] = { keys[i].symbol };
      XChangeKeyboardMapping(xdo->xdpy, scratch_keycode, 1, keysym_list, 1);
      /* override the code in our current key to use the scratch_keycode */
      keys[i].code = scratch_keycode;
      keymapchanged = 1;
    }

    //printf("Sending %d (mods %x)\n", keys[i].code, *modifier);
    _xdo_send_key(xdo, window, keys[i].code, *modifier, pressed, 12000);

    if (keys[i].needs_binding == 1) {
      /* If we needed to make a new keymapping for this keystroke, we
       * should sync with the server now, after the keypress, so that
       * the next mapping or removal doesn't conflict. */
      XSync(xdo->xdpy, False);
    }

    if (pressed) {
      *modifier |= _xdo_cached_keycode_to_modifier(xdo, keys[i].code);
    } else {
      *modifier &= ~(_xdo_cached_keycode_to_modifier(xdo, keys[i].code));
    }
  }


  if (keymapchanged) {
    KeySym keysym_list[] = { 0 };
    XChangeKeyboardMapping(xdo->xdpy, scratch_keycode, 1, keysym_list, 1);
  }

  /* Necessary? */
  XFlush(xdo->xdpy);
  return XDO_SUCCESS;
}

  
int xdo_keysequence_down(const xdo_t *xdo, Window window, const char *keyseq) {
  return _xdo_keysequence_do(xdo, window, keyseq, True, NULL);
}

int xdo_keysequence_up(const xdo_t *xdo, Window window, const char *keyseq) {
  return _xdo_keysequence_do(xdo, window, keyseq, False, NULL);
}

int xdo_keysequence(const xdo_t *xdo, Window window, const char *keyseq) {
  int ret = 0;
  int modifier = 0;
  ret += _xdo_keysequence_do(xdo, window, keyseq, True, &modifier);
  ret += _xdo_keysequence_do(xdo, window, keyseq, False, &modifier);
  return ret;
}

/* Add by Lee Pumphret 2007-07-28
 * Modified slightly by Jordan Sissel */
int xdo_window_get_focus(const xdo_t *xdo, Window *window_ret) {
  int ret = 0;
  int unused_revert_ret;

  ret = XGetInputFocus(xdo->xdpy, window_ret, &unused_revert_ret);
  return _is_success("XGetInputFocus", ret == 0);
}

/* Like xdo_window_get_focus, but return the first ancestor-or-self window
 * having a property of WM_CLASS. This allows you to get the "real" or
 * top-level-ish window having focus rather than something you may
 * not expect to be the window having focused. */
int xdo_window_sane_get_focus(const xdo_t *xdo, Window *window_ret) {
  int done = 0;
  Window w;
  XClassHint classhint;

  /* for XQueryTree */
  Window dummy, parent, *children = NULL;
  unsigned int nchildren;

  xdo_window_get_focus(xdo, &w);

  while (!done) {
    Status s;
    s = XGetClassHint(xdo->xdpy, w, &classhint);
    //fprintf(stderr, "%d\n", s);

    if (s == 0) {
      /* Error. This window doesn't have a class hint */
      //fprintf(stderr, "no class on: %d\n", w);
      XQueryTree(xdo->xdpy, w, &dummy, &parent, &children, &nchildren);

      /* Don't care about the children, but we still need to free them */
      if (children != NULL)
        XFree(children);
      //fprintf(stderr, "parent: %d\n", parent);
      w = parent;
    } else {
      /* Must XFree the class and name items. */
      XFree(classhint.res_class);
      XFree(classhint.res_name);

      done = 1;
    }
  }

  *window_ret = w;
  return _is_success("xdo_window_sane_get_focus", w == 0);
}

/* Helper functions */
static int _xdo_keycode_from_char(const xdo_t *xdo, char key) {
  int i = 0;
  int len = xdo->keycode_high - xdo->keycode_low;

  for (i = 0; i < len; i++)
    if (xdo->charcodes[i].key == key)
      return xdo->charcodes[i].code;

  return -1;
}

static int _xdo_get_shiftcode_if_needed(const xdo_t *xdo, char key) {
  int i = 0;
  int len = xdo->keycode_high - xdo->keycode_low;

  for (i = 0; i < len; i++)
    if (xdo->charcodes[i].key == key)
      return xdo->charcodes[i].shift;

  return -1;
}

static int _xdo_has_xtest(const xdo_t *xdo) {
  int dummy;
  return (XTestQueryExtension(xdo->xdpy, &dummy, &dummy, &dummy, &dummy) == True);
}

static void _xdo_populate_charcode_map(xdo_t *xdo) {
  /* assert xdo->display is valid */
  int keycodes_length = 0;
  int shift_keycode = 0;
  int i, j;

  XDisplayKeycodes(xdo->xdpy, &(xdo->keycode_low), &(xdo->keycode_high));

  /* Double size of keycode range because some 
   * keys have "shift" values. ie; 'a' and 'A', '2' and '@' */
  /* Add 2 to the size because the range [low, high] is inclusive */
  /* Add 2 more for tab (\t) and newline (\n) */
  keycodes_length = (xdo->keycode_high - xdo->keycode_low) * 2 + (2 + 2);
  xdo->charcodes = malloc(keycodes_length * sizeof(charcodemap_t));
  memset(xdo->charcodes, 0, keycodes_length * sizeof(charcodemap_t));

  /* Fetch the keycode for Shift_L */
  /* XXX: Make 'Shift_L' configurable? */
  shift_keycode = XKeysymToKeycode(xdo->xdpy, XK_Shift_L);

  for (i = xdo->keycode_low; i <= xdo->keycode_high; i++) {
    char *keybuf = 0;

    /* Index '0' in KeycodeToKeysym == no shift key
     * Index '1' in ... == shift key held
     * hence this little loop. */
    for (j = 0; j <= 1; j++) { 
     int idx = (i - xdo->keycode_low) * 2 + j;
     keybuf = XKeysymToString(XKeycodeToKeysym(xdo->xdpy, i, j));

     xdo->charcodes[idx].key = _keysym_to_char(keybuf);
     xdo->charcodes[idx].code = i;
     xdo->charcodes[idx].shift = j ? shift_keycode : 0;
     xdo->charcodes[idx].modmask = _xdo_query_keycode_to_modifier(xdo, i);
    }
  }

  /* Add special handling so we can translate ASCII newline and tab
   * to keycodes */
  j = (xdo->keycode_high - xdo->keycode_low) * 2;
  xdo->charcodes[j].key = '\n';
  xdo->charcodes[j].code = XKeysymToKeycode(xdo->xdpy, XK_Return);
  xdo->charcodes[j].shift = 0;
  xdo->charcodes[j].modmask = 0;

  j++;
  xdo->charcodes[j].key = '\t';
  xdo->charcodes[j].code = XKeysymToKeycode(xdo->xdpy, XK_Tab);
  xdo->charcodes[j].shift = 0;
  xdo->charcodes[j].modmask = 0;
}

/* context-free functions */
char _keysym_to_char(const char *keysym) {
  int i;

  if (keysym == NULL)
    return -1;

  /* keysym_charmap comes from xdo_util.h */
  for (i = 0; keysym_charmap[i].keysym; i++) {
    if (!strcmp(keysym_charmap[i].keysym, keysym))
      return keysym_charmap[i].key;
  }

  if (strlen(keysym) == 1)
    return keysym[0];

  return -1;
}

int _xdo_keysequence_to_keycode_list(const xdo_t *xdo, const char *keyseq,
                                     charcodemap_t **keys, int *nkeys) {
  char *tokctx = NULL;
  const char *tok = NULL;
  char *keyseq_copy = NULL, *strptr = NULL;
  int i;
  int shift_keycode;
  
  /* Array of keys to press, in order given by keyseq */
  int keys_size = 10;
  *nkeys = 0;

  if (strcspn(keyseq, " \t\n.-[]{}\\|") != strlen(keyseq)) {
    fprintf(stderr, "Error: Invalid key sequence '%s'\n", keyseq);
    return False;
  }

  shift_keycode = XKeysymToKeycode(xdo->xdpy, XStringToKeysym("Shift_L"));

  *keys = malloc(keys_size * sizeof(charcodemap_t));
  keyseq_copy = strptr = strdup(keyseq);
  while ((tok = strtok_r(strptr, "+", &tokctx)) != NULL) {
    KeySym sym;
    KeyCode key;

    if (strptr != NULL)
      strptr = NULL;

    /* Check if 'tok' (string keysym) is an alias to another key */
    /* symbol_map comes from xdo.util */
    for (i = 0; symbol_map[i] != NULL; i+=2)
      if (!strcasecmp(tok, symbol_map[i]))
        tok = symbol_map[i + 1];

    sym = XStringToKeysym(tok);
    if (sym == NoSymbol) {
      /* Accept a number as a explicit keycode */
      if (isdigit(tok[0])) {
        key = (unsigned int) atoi(tok);
      } else {
        fprintf(stderr, "(symbol) No such key name '%s'. Ignoring it.\n", tok);
        continue;
      }
    } else {
      key = XKeysymToKeycode(xdo->xdpy, sym);
    }

    if (key == 0) {
      //fprintf(stderr, "No such key '%s'. Ignoring it.\n", tok);
      (*keys)[*nkeys].symbol = sym;
      (*keys)[*nkeys].needs_binding = 1;
    } else {
      (*keys)[*nkeys].symbol = 0;
      (*keys)[*nkeys].needs_binding = 0;
      (*keys)[*nkeys].code = key;
      if ((XKeycodeToKeysym(xdo->xdpy, key, 0) == sym)
          || sym == NoSymbol) {
        /* sym is NoSymbol if we give a keycode to type */
        (*keys)[*nkeys].shift = 0;
      } else  {
        /* Inject a 'shift' key item if we should be using shift */
        //fprintf(stderr, "Key '%s' doesn't match first symbol\n", tok);
        (*keys)[*nkeys].code = shift_keycode;
        (*keys)[*nkeys].shift = 0;

        (*nkeys)++;
        if (*nkeys == keys_size) {
          keys_size *= 2;
          *keys = realloc(*keys, keys_size * sizeof(charcodemap_t));
        }
        (*keys)[*nkeys].shift = shift_keycode;
      }
    }

    (*nkeys)++;
    if (*nkeys == keys_size) {
      keys_size *= 2;
      *keys = realloc(*keys, keys_size * sizeof(KeyCode));
    }
  }

  free(keyseq_copy);
  return True;
}

int _is_success(const char *funcname, int code) {
  /* Nonzero is failure. */
  if (code != 0)
    fprintf(stderr, "%s failed (code=%d)\n", funcname, code);
  return code;
}

/* Arbitrary window property retrieval
 * slightly modified version from xprop.c from Xorg */
unsigned char *xdo_getwinprop(const xdo_t *xdo, Window window, Atom atom,
                              long *nitems, Atom *type, int *size) {
  Atom actual_type;
  int actual_format;
  unsigned long _nitems;
  unsigned long nbytes;
  unsigned long bytes_after; /* unused */
  unsigned char *prop;
  int status;

  status = XGetWindowProperty(xdo->xdpy, window, atom, 0, (~0L),
                              False, AnyPropertyType, &actual_type,
                              &actual_format, &_nitems, &bytes_after,
                              &prop);
  if (status == BadWindow) {
    fprintf(stderr, "window id # 0x%lx does not exists!", window);
    return NULL;
  } if (status != Success) {
    fprintf(stderr, "XGetWindowProperty failed!");
    return NULL;
  }

  if (actual_format == 32)
    nbytes = sizeof(long);
  else if (actual_format == 16)
    nbytes = sizeof(short);
  else if (actual_format == 8)
    nbytes = 1;
  else if (actual_format == 0)
    nbytes = 0;

  *nitems = _nitems;
  *type = actual_type;
  *size = actual_format;
  return prop;
}

int _xdo_ewmh_is_supported(const xdo_t *xdo, const char *feature) {
  Atom type = 0;
  long nitems = 0L;
  int size = 0;
  Atom *results = NULL;
  long i = 0;

  Window root;
  Atom request;
  Atom feature_atom;
  
  request = XInternAtom(xdo->xdpy, "_NET_SUPPORTED", False);
  feature_atom = XInternAtom(xdo->xdpy, feature, False);
  root = RootWindow(xdo->xdpy, 0);

  results = (Atom *) xdo_getwinprop(xdo, root, request, &nitems, &type, &size);
  for (i = 0L; i < nitems; i++) {
    if (results[i] == feature_atom)
      return True;
  }
  free(results);

  return False;
}

void _xdo_init_xkeyevent(const xdo_t *xdo, XKeyEvent *xk) {
  xk->display = xdo->xdpy;
  xk->subwindow = None;
  xk->time = CurrentTime;
  xk->same_screen = True;

  /* Should we set these at all? */
  xk->x = xk->y = xk->x_root = xk->y_root = 1;
}

void _xdo_send_key(const xdo_t *xdo, Window window, int keycode, int modstate,
                   int is_press, useconds_t delay) {
  if (window == CURRENTWINDOW) {
    /* Properly ensure the modstate is set by finding a key
     * that activates each bit in the modifier state */
    int masks[] = { ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask,
                    Mod3Mask, Mod4Mask, Mod5Mask };
    unsigned int i = 0;
    if (modstate != 0) {
      for (i = 0; i < 8; i++) { /* 8 == number of masks */
        if (modstate & masks[i]) {
          KeyCode key;
          key = _xdo_cached_modifier_to_keycode(xdo, masks[i]),
          XTestFakeKeyEvent(xdo->xdpy, key, is_press, CurrentTime);
        }
      }
    }

    XTestFakeKeyEvent(xdo->xdpy, keycode, is_press, CurrentTime);
  } else {
    /* Since key events have 'state' (shift, etc) in the event, we don't
     * need to worry about key press ordering. */
    XKeyEvent xk;
    _xdo_init_xkeyevent(xdo, &xk);
    xk.window = window;
    xk.keycode = keycode;
    xk.state = modstate;
    xk.type = (is_press ? KeyPress : KeyRelease);
    XSendEvent(xdo->xdpy, xk.window, True, KeyPressMask, (XEvent *)&xk);
  }

  /* Skipping the usleep if delay is 0 is much faster than calling usleep(0) */
  XFlush(xdo->xdpy);
  if (delay > 0) {
    usleep(delay);
  }
}

int _xdo_query_keycode_to_modifier(const xdo_t *xdo, KeyCode keycode) {
  int i = 0, j = 0;
  int max = xdo->modmap->max_keypermod;

  for (i = 0; i < 8; i++) { /* 8 modifier types, per XGetModifierMapping(3X) */
    for (j = 0; j < max && xdo->modmap->modifiermap[(i * max) + j]; j++) {
      if (keycode == xdo->modmap->modifiermap[(i * max) + j]) {
        switch (i) {
          case ShiftMapIndex: return ShiftMask; break;
          case LockMapIndex: return LockMask; break;
          case ControlMapIndex: return ControlMask; break;
          case Mod1MapIndex: return Mod1Mask; break;
          case Mod2MapIndex: return Mod2Mask; break;
          case Mod3MapIndex: return Mod3Mask; break;
          case Mod4MapIndex: return Mod4Mask; break;
          case Mod5MapIndex: return Mod5Mask; break;
        }
      } /* end if */
    } /* end loop j */
  } /* end loop i */

  /* No modifier found for this keycode, return no mask */
  return 0;
}

int _xdo_cached_keycode_to_modifier(const xdo_t *xdo, KeyCode keycode) {
  int i = 0;
  int len = xdo->keycode_high - xdo->keycode_low;

  for (i = 0; i < len; i++)
    if (xdo->charcodes[i].code == keycode)
      return xdo->charcodes[i].modmask;

  return 0;
}

int _xdo_cached_modifier_to_keycode(const xdo_t *xdo, int modmask) {
  int i = 0;
  int len = xdo->keycode_high - xdo->keycode_low;

  for (i = 0; i < len; i++)
    if (xdo->charcodes[i].modmask == modmask)
      return xdo->charcodes[i].code;

  return 0;
}

int xdo_active_keys_to_keycode_list(const xdo_t *xdo, charcodemap_t **keys,
                                          int *nkeys) {
  /* For each keyboard device, if an active key is a modifier,
   * then add the keycode to the keycode list */

  char keymap[32]; /* keycode map: 256 bits */
  int keys_size = 10;
  int keycode = 0;
  *nkeys = 0;
  *keys = malloc(keys_size * sizeof(charcodemap_t));

  XQueryKeymap(xdo->xdpy, keymap);

  for (keycode = xdo->keycode_low; keycode <= xdo->keycode_high; keycode++) {
    if ((keymap[(keycode / 8)] & (1 << (keycode % 8))) \
        && _xdo_cached_keycode_to_modifier(xdo, keycode)) {
      /* This keycode is active and is a modifier */

      (*keys)[*nkeys].code = keycode;
      (*nkeys)++;

      if (*nkeys == keys_size) {
        keys_size *= 2;
        *keys = malloc(keys_size * sizeof(charcodemap_t));
      }
    }
  } 

  return XDO_SUCCESS;
}

unsigned int xdo_get_input_state(const xdo_t *xdo) {
  Window root, dummy;
  int root_x, root_y, win_x, win_y;
  unsigned int mask;
  root = DefaultRootWindow(xdo->xdpy);

  XQueryPointer(xdo->xdpy, root, &dummy, &dummy,
                &root_x, &root_y, &win_x, &win_y, &mask);

  return mask;
}

const keysym_charmap_t *xdo_keysym_charmap(void) {
  return keysym_charmap;
}

const char **xdo_symbol_map(void) {
  return symbol_map;
}

xdo_active_mods_t *xdo_get_active_modifiers(const xdo_t *xdo) {
  xdo_active_mods_t *active_mods = NULL;

  active_mods = calloc(sizeof(xdo_active_mods_t), 1);
  xdo_active_keys_to_keycode_list(xdo, &(active_mods->keymods),
                                       &(active_mods->nkeymods));
  active_mods->input_state = xdo_get_input_state(xdo);
  return active_mods;
}

int xdo_clear_active_modifiers(const xdo_t *xdo, Window window, xdo_active_mods_t *active_mods) {
  int ret = 0;
  xdo_keysequence_list_do(xdo, window, active_mods->keymods,
                          active_mods->nkeymods, False, NULL);

  if (active_mods->input_state & Button1MotionMask)
    ret = xdo_mouseup(xdo, window, 1);
  if (!ret && active_mods->input_state & Button2MotionMask)
    ret = xdo_mouseup(xdo, window, 2);
  if (!ret && active_mods->input_state & Button3MotionMask)
    ret = xdo_mouseup(xdo, window, 3);
  if (!ret && active_mods->input_state & Button4MotionMask)
    ret = xdo_mouseup(xdo, window, 4);
  if (!ret && active_mods->input_state & Button5MotionMask)
    ret = xdo_mouseup(xdo, window, 5);
  if (!ret && active_mods->input_state & LockMask) {
    /* explicitly use down+up here since xdo_keysequence alone will track the modifiers
     * incurred by a key (like shift, or caps) and send them on the 'up' sequence.
     * That seems to break things with Caps_Lock only, so let's be explicit here. */
    ret = xdo_keysequence_down(xdo, window, "Caps_Lock");
    ret += xdo_keysequence_up(xdo, window, "Caps_Lock");
  }

  XSync(xdo->xdpy, False);
  return ret;
}

int xdo_set_active_modifiers(const xdo_t *xdo, Window window, const xdo_active_mods_t *active_mods) {
  int ret = 0;
  xdo_keysequence_list_do(xdo, window, active_mods->keymods,
                          active_mods->nkeymods, True, NULL);
  if (active_mods->input_state & Button1MotionMask)
    ret = xdo_mousedown(xdo, window, 1);
  if (!ret && active_mods->input_state & Button2MotionMask)
    ret = xdo_mousedown(xdo, window, 2);
  if (!ret && active_mods->input_state & Button3MotionMask)
    ret = xdo_mousedown(xdo, window, 3);
  if (!ret && active_mods->input_state & Button4MotionMask)
    ret = xdo_mousedown(xdo, window, 4);
  if (!ret && active_mods->input_state & Button5MotionMask)
    ret = xdo_mousedown(xdo, window, 5);
  if (!ret && active_mods->input_state & LockMask) {
    /* explicitly use down+up here since xdo_keysequence alone will track the modifiers
     * incurred by a key (like shift, or caps) and send them on the 'up' sequence.
     * That seems to break things with Caps_Lock only, so let's be explicit here. */
    ret = xdo_keysequence_down(xdo, window, "Caps_Lock");
    ret += xdo_keysequence_up(xdo, window, "Caps_Lock");
  }

  XSync(xdo->xdpy, False);
  return ret;
}

void xdo_free_active_modifiers(xdo_active_mods_t *active_mods) {
  free(active_mods->keymods);
  free(active_mods);
}

