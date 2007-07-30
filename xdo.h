/*
 * xdo library header
 * - include this if you have code that uses xdo
 *
 * $Id$
 */

#include <X11/Xlib.h>

#define SEARCH_VISIBLEONLY (1L << 0)
#define SEARCH_TITLE (1L << 1)
#define SEARCH_CLASS (1L << 2)
#define SEARCH_NAME (1L << 3)

#define SIZE_USEHINTS (1L << 0)

/* Map keysym name to actual ascii */
typedef struct keysymcharmap {
  char *keysym;
  char key;
} keysymcharmap_t;

/* map character to keycode */
typedef struct charcodemap {
  char key;
  int code;
  int shift;
} charcodemap_t;

typedef struct xdo {
  Display *xdpy;
  char *display_name;
  charcodemap_t *charcodes;
  int keycode_high; /* highest and lowest keycodes */
  int keycode_low;  /* used by this X server */

  int close_display_when_freed;
} xdo_t;


xdo_t* xdo_new(char *display);
xdo_t* xdo_new_with_opened_display(Display *xdpy, char *display,
                                   int close_display_when_freed);
void xdo_free(xdo_t *xdo);

int xdo_mousemove(xdo_t *xdo, int x, int y);
//void xdo_motion_relative(xdo_t *xdo, int x, int y);
int xdo_mousedown(xdo_t *xdo, int button);
int xdo_mouseup(xdo_t *xdo, int button);

int xdo_click(xdo_t *xdo, int button);

int xdo_type(xdo_t *xdo, char *string);
int xdo_keysequence(xdo_t *xdo, char *keysequence);

int xdo_window_move(xdo_t *xdo, int wid, int x, int y);
int xdo_window_setsize(xdo_t *xdo, int wid, int w, int h, int flags);
int xdo_window_focus(xdo_t *xdo, int wid);
int xdo_window_raise(xdo_t *xdo, int wid);
int xdo_window_get_focus(xdo_t *xdo, int *window_ret);

int xdo_window_map(xdo_t *xdo, int wid);
int xdo_window_unmap(xdo_t *xdo, int wid);

/* Returns: windowlist and nwindows */
void xdo_window_list_by_regex(xdo_t *xdo, char *regex, int flags,
                              Window **windowlist, int *nwindows);
