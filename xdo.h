/*
 * xdo library header
 * - include this if you have code that uses xdo
 *
 */
#ifndef _XDO_H_
#define _XDO_H_

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif /* __USE_XOPEN */

#include <sys/types.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define SEARCH_IGNORE_TRANSIENTS (1L << 4)
#define SEARCH_IGNORE_WINDOW_INPUTONLY (1L << 5)

#define SIZE_USEHINTS (1L << 0)
#define CURRENTWINDOW (0)

/* Map keysym name to actual ascii */
typedef struct keysym_charmap {
  const char *keysym;
  char key;
} keysym_charmap_t;

/* map character to keycode */
typedef struct charcodemap {
  char key;
  KeyCode code;
  KeySym symbol;
  int shift;
  int modmask;
  int needs_binding;
} charcodemap_t;

typedef struct xdo {
  Display *xdpy;
  char *display_name;
  charcodemap_t *charcodes;
  XModifierKeymap *modmap;
  int keycode_high; /* highest and lowest keycodes */
  int keycode_low;  /* used by this X server */

  int close_display_when_freed;
} xdo_t;

typedef struct xdo_active_mods {
  charcodemap_t *keymods;
  int nkeymods;
  unsigned int input_state;
} xdo_active_mods_t;

#define SEARCH_TITLE (1UL << 0)
#define SEARCH_CLASS (1UL << 1)
#define SEARCH_NAME (1UL << 2)
#define SEARCH_PID  (1UL << 3)
#define SEARCH_ONLYVISIBLE  (1UL << 4)
#define SEARCH_SCREEN  (1UL << 5)

typedef struct xdo_search {
  char *title;       /* pattern to test against a window title */
  char *winclass;    /* pattern to test against a window class */
  char *winname;     /* pattern to test against a window name */
  unsigned long pid; /* window pid (From window atom _NET_WM_PID) */
  long max_depth;    /* depth of search. 1 means only toplevel windows */
  int only_visible;  /* boolean; set true to search only visible windows */
  int screen;        /* what screen to search, if any. If none given, search 
                        all screens */

  /* Should the tests be 'and' or 'or' ? If 'and', any failure will skip the
   * window. If 'or', any success will keep the window in search results. */
  enum { SEARCH_ANY, SEARCH_ALL } require;
  unsigned int searchmask; /* bitmask of things you are searching for */
} xdo_search_t;

#define XDO_ERROR 1
#define XDO_SUCCESS 0

xdo_t* xdo_new(char *display);
xdo_t* xdo_new_with_opened_display(Display *xdpy, const char *display,
                                   int close_display_when_freed);

const char *xdo_version(void);
void xdo_free(xdo_t *xdo);

int xdo_mousemove(const xdo_t *xdo, int x, int y);
int xdo_mousemove_relative(const xdo_t *xdo, int x, int y);
int xdo_mousedown(const xdo_t *xdo, Window window, int button);
int xdo_mouseup(const xdo_t *xdo, Window window, int button);
int xdo_mouselocation(const xdo_t *xdo, int *x, int *y, int *screen_num);

int xdo_click(const xdo_t *xdo, Window window, int button);

int xdo_type(const xdo_t *xdo, Window window, char *string, useconds_t delay);
int xdo_keysequence(const xdo_t *xdo, Window window, const char *keysequence);
int xdo_keysequence_up(const xdo_t *xdo, Window window, const char *keysequence);
int xdo_keysequence_down(const xdo_t *xdo, Window window, const char *keysequence);
int xdo_keysequence_list_do(const xdo_t *xdo, Window window, charcodemap_t *keys,
                             int nkeys, int pressed, int *modifier);
int xdo_active_keys_to_keycode_list(const xdo_t *xdo, charcodemap_t **keys,
                                         int *nkeys);

int xdo_window_move(const xdo_t *xdo, Window wid, int x, int y);
int xdo_window_setsize(const xdo_t *xdo, Window wid, int w, int h, int flags);
int xdo_window_setprop (const xdo_t *xdo, Window wid, const char *property, const char *value);
int xdo_window_setclass(const xdo_t *xdo, Window wid, const char *name, const char *class);
int xdo_window_focus(const xdo_t *xdo, Window wid);
int xdo_window_raise(const xdo_t *xdo, Window wid);
int xdo_window_get_focus(const xdo_t *xdo, Window *window_ret);
int xdo_window_sane_get_focus(const xdo_t *xdo, Window *window_ret);
int xdo_window_activate(const xdo_t *xdo, Window wid);

int xdo_window_map(const xdo_t *xdo, Window wid);
int xdo_window_unmap(const xdo_t *xdo, Window wid);

/* pager-like behaviors */
int xdo_window_get_active(const xdo_t *xdo, Window *window_ret);
int xdo_set_number_of_desktops(const xdo_t *xdo, long ndesktops);
int xdo_get_number_of_desktops(const xdo_t *xdo, long *ndesktops);
int xdo_set_current_desktop(const xdo_t *xdo, long desktop);
int xdo_get_current_desktop(const xdo_t *xdo, long *desktop);
int xdo_set_desktop_for_window(const xdo_t *xdo, Window wid, long desktop);
int xdo_get_desktop_for_window(const xdo_t *xdo, Window wid, long *desktop);

int xdo_window_search(const xdo_t *xdo, const xdo_search_t *search,
                      Window **windowlist_ret, int *nwindows_ret);

unsigned char *xdo_getwinprop(const xdo_t *xdo, Window window, Atom atom,
                              long *nitems, Atom *type, int *size);
unsigned int xdo_get_input_state(const xdo_t *xdo);
const keysym_charmap_t *xdo_keysym_charmap(void);
const char **xdo_symbol_map(void);

/* active modifiers stuff */
xdo_active_mods_t *xdo_get_active_modifiers(const xdo_t *xdo);
int xdo_clear_active_modifiers(const xdo_t *xdo, Window window, xdo_active_mods_t *active_mods);
int xdo_set_active_modifiers(const xdo_t *xdo, Window window, const xdo_active_mods_t *active_mods);
void xdo_free_active_modifiers(xdo_active_mods_t *active_mods);

#endif /* ifndef _XDO_H_ */
