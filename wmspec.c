
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <stdlib.h>

unsigned char * get_window_property(Display *xdpy, Window window, Atom atom, 
                                    long *nitems, Atom *type, int *size) {
  /* slightly modified version from xprop.c from Xorg */
  Atom actual_type;
  int actual_format;
  unsigned long _nitems;
  unsigned long nbytes;
  unsigned long bytes_after; /* unused */
  unsigned char *prop;
  int status;

  status = XGetWindowProperty(xdpy, window, atom, 0, (~0L),
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

int *ewmh__NET_SUPPORTED(Display *xdpy, Window wid, Atom **atoms, long *nitems) {
  unsigned char *data;
  long _nitems;
  Atom type;
  int size;
  data = get_window_property(xdpy, wid, 
                             XInternAtom(xdpy, "_NET_SUPPORTED", False),
                             &_nitems, &type, &size);
  if (nitems <= 0)
    return NULL;

  *atoms = (Atom *)data;
  *nitems = _nitems;
}

int main(int argc, char **argv) {
  Display *xdpy;
  Window root;
  long length;
  Atom type;
  int size;
  unsigned char *data;

  xdpy = XOpenDisplay(NULL);
  root = RootWindow(xdpy, 0);
  data = get_window_property(xdpy, root,
                             XInternAtom(xdpy, argv[1], False),
                             &length, &type, &size);
  printf("Length: %ld\n", length);
  printf("Type: %ld\n", type);
  printf("Size: %d\n", size);

  Atom *supported;
  long nitems;
  ewmh__NET_SUPPORTED(xdpy, root, &supported, &nitems);

  XFree(data);

  return 0;
}

