/**
 * @file xdo.h
 */
#ifndef _XDO_H_
#define _XDO_H_

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif /* __USE_XOPEN */

#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <unistd.h>

/**
 * @mainpage
 *
 * libxdo helps you send fake mouse and keyboard input, search for windows,
 * perform various window managment tasks such as desktop changes, window
 * movement, etc.
 *
 * For examples on libxdo usage, the xdotool source code is a good reference.
 *
 * @see xdo.h
 * @see xdo_new
 */

/**
 * When issuing a window size change, giving this flag will make the size
 * change be relative to the size hints of the window.  For terminals, this
 * generally means that the window size will be relative to the font size,
 * allowing you to change window sizes based on character rows and columns
 * instead of pixels.
 */
#define SIZE_USEHINTS (1L << 0)
#define SIZE_USEHINTS_X (1L << 1)
#define SIZE_USEHINTS_Y (1L << 2)

/**
 * CURRENTWINDOW is a special identify for xdo input faking (mouse and
 * keyboard) functions like xdo_keysequence that indicate we should target the
 * current window, not a specific window.
 *
 * Generally, this means we will use XTEST instead of XSendEvent when sending
 * events.
 */
#define CURRENTWINDOW (0)

/**
 * @internal
 * Map keysym name to actual ascii 
 */
typedef struct keysym_charmap {
  const char *keysym;
  char key;
} keysym_charmap_t;

/**
 * @internal
 * Map character to whatever information we need to be able to send
 * this key (keycode, modifiers, key index, etc)
 */
typedef struct charcodemap {
  char key; /** the letter for this key, like 'a' */
  KeyCode code; /** the keycode that this key is on */
  KeySym symbol; /** the symbol representing this key */
  int index; /** the index in the keysym-per-keycode list that is this key */
  int modmask; /** the modifiers activated by this key */
   /** if this key need to be bound at runtime because it does not
    * exist in the current keymap, this will be set to 1. */
  int needs_binding;
} charcodemap_t;

/**
 * The main context.
 */
typedef struct xdo {

  /** The Display for Xlib */
  Display *xdpy;

  /** The display name, if any. NULL if not specified. */
  char *display_name;

  /** @internal Array of known keys/characters */
  charcodemap_t *charcodes;

  /** @internal Lenth of charcodes array */
  int charcodes_len;

  /** @internal result from XGetModifierMapping */
  XModifierKeymap *modmap;

  /** @internal current keyboard mapping (via XGetKeyboardMapping) */
  KeySym *keymap;

  /** @internal highest keycode value */
  int keycode_high; /* highest and lowest keycodes */

  /** @internal lowest keycode value */
  int keycode_low;  /* used by this X server */
  
  /** @internal number of keysyms per keycode */
  int keysyms_per_keycode;

  /** Should we close the display when calling xdo_free? */
  int close_display_when_freed;
} xdo_t;


/**
 * Represents the list of active modifier keys at the time of an
 * xdo_get_active_modifiers call.
 *
 * @see xdo_get_active_modifiers
 * @see xdo_free_active_modifiers
 */
typedef struct xdo_active_mods {
  charcodemap_t *keymods;
  int nkeymods;
  unsigned int input_state;
} xdo_active_mods_t;


/**
 * Search only window title. DEPRECATED - Use SEARCH_NAME
 * @see xdo_window_search
 */
#define SEARCH_TITLE (1UL << 0)

/**
 * Search only window class.
 * @see xdo_window_search
 */
#define SEARCH_CLASS (1UL << 1)

/**
 * Search only window name.
 * @see xdo_window_search
 */
#define SEARCH_NAME (1UL << 2)

/**
 * Search only window pid.
 * @see xdo_window_search
 */
#define SEARCH_PID  (1UL << 3)

/**
 * Search only visible windows.
 * @see xdo_window_search
 */
#define SEARCH_ONLYVISIBLE  (1UL << 4)

/**
 * Search only a specific screen. 
 * @see xdo_search.screen
 * @see xdo_window_search
 */
#define SEARCH_SCREEN  (1UL << 5)

/**
 * Search only window class name.
 * @see xdo_search
 */
#define SEARCH_CLASSNAME (1UL << 6)


/**
 * The window search query structure.
 *
 * @see xdo_window_search
 */
typedef struct xdo_search {
  const char *title;        /** pattern to test against a window title */
  const char *winclass;     /** pattern to test against a window class */
  const char *winclassname; /** pattern to test against a window class */
  const char *winname;      /** pattern to test against a window name */
  int pid;            /** window pid (From window atom _NET_WM_PID) */
  long max_depth;     /** depth of search. 1 means only toplevel windows */
  int only_visible;   /** boolean; set true to search only visible windows */
  int screen;         /** what screen to search, if any. If none given, search 
                         all screens */

  /** Should the tests be 'and' or 'or' ? If 'and', any failure will skip the
   * window. If 'or', any success will keep the window in search results. */
  enum { SEARCH_ANY, SEARCH_ALL } require;
  
  /** bitmask of things you are searching for, such as SEARCH_NAME, etc.
   * @see SEARCH_NAME, SEARCH_CLASS, SEARCH_PID, SEARCH_CLASSNAME, etc
   */
  unsigned int searchmask; 
} xdo_search_t;

#define XDO_ERROR 1
#define XDO_SUCCESS 0

/**
 * Create a new xdo_t instance.
 *
 * @param display the string display name, such as ":0". If null, uses the
 * environment variable DISPLAY just like XOpenDisplay(NULL).
 *
 * @return Pointer to a new xdo_t or NULL on failure
 */
xdo_t* xdo_new(char *display);

/**
 * Create a new xdo_t instance with an existing X11 Display instance.
 *
 * @param xdpy the Display pointer given by a previous XOpenDisplay()
 * @param display the string display name
 * @param close_display_when_freed If true, we will close the display when
 * xdo_free is called. Otherwise, we leave it open.
 */
xdo_t* xdo_new_with_opened_display(Display *xdpy, const char *display,
                                   int close_display_when_freed);

/**
 * Return a string representing the version of this library
 */
const char *xdo_version(void);

/**
 * Free and destroy an xdo_t instance.
 *
 * If close_display_when_freed is set, then we will also close the Display.
 */
void xdo_free(xdo_t *xdo);

/**
 * Move the mouse to a specific location.
 *
 * @param x the target X coordinate on the screen in pixels.
 * @param y the target Y coordinate on the screen in pixels.
 * @param screen the screen (number) you want to move on.
 */
int xdo_mousemove(const xdo_t *xdo, int x, int y, int screen);

/**
 * Move the mouse to a specific location relative to the top-left corner
 * of a window.
 *
 * @param x the target X coordinate on the screen in pixels.
 * @param y the target Y coordinate on the screen in pixels.
 */
int xdo_mousemove_relative_to_window(const xdo_t *xdo, Window window, int x, int y);

/**
 * Move the mouse relative to it's current position.
 *
 * @param x the distance in pixels to move on the X axis.
 * @param y the distance in pixels to move on the Y axis.
 */
int xdo_mousemove_relative(const xdo_t *xdo, int x, int y);

/**
 * Send a mouse press (aka mouse down) for a given button at the current mouse
 * location.
 *
 * @param window The window you want to send the event to or CURRENTWINDOW
 * @param button The mouse button. Generally, 1 is left, 2 is middle, 3 is
 *    right, 4 is wheel up, 5 is wheel down.
 */
int xdo_mousedown(const xdo_t *xdo, Window window, int button);

/**
 * Send a mouse release (aka mouse up) for a given button at the current mouse
 * location.
 *
 * @param window The window you want to send the event to or CURRENTWINDOW
 * @param button The mouse button. Generally, 1 is left, 2 is middle, 3 is
 *    right, 4 is wheel up, 5 is wheel down.
 */
int xdo_mouseup(const xdo_t *xdo, Window window, int button);

/**
 * Get the current mouse location (coordinates and screen number).
 *
 * @param x integer pointer where the X coordinate will be stored
 * @param y integer pointer where the Y coordinate will be stored
 * @param screen_num integer pointer where the screen number will be stored
 */
int xdo_mouselocation(const xdo_t *xdo, int *x, int *y, int *screen_num);

/**
 * Get the window the mouse is currently over
 *
 * @param window_ret Winter pointer where the window will be stored.
 */
int xdo_mousewindow(const xdo_t *xdo, Window *window_ret);

/**
 * Get all mouse location-related data.
 *
 * If null is passed for any parameter, we simply do not store it.
 * Useful if you only want the 'y' coordinate, for example.
 *
 * @param x integer pointer where the X coordinate will be stored
 * @param y integer pointer where the Y coordinate will be stored
 * @param screen_num integer pointer where the screen number will be stored
 * @param window Window pointer where the window/client the mouse is over
 *   will be stored.
 */
int xdo_mouselocation2(const xdo_t *xdo, int *x_ret, int *y_ret,
                       int *screen_num_ret, Window *window_ret);

/**
 * Wait for the mouse to move from a location. This function will block
 * until the condition has been satisified.
 *
 * @param origin_x the X position you expect the mouse to move from
 * @param origin_y the Y position you expect the mouse to move from
 */
int xdo_mouse_wait_for_move_from(const xdo_t *xdo, int origin_x, int origin_y);

/**
 * Wait for the mouse to move to a location. This function will block
 * until the condition has been satisfied.
 *
 * @param dest_x the X position you expect the mouse to move to
 * @param dest_y the Y position you expect the mouse to move to
 */
int xdo_mouse_wait_for_move_to(const xdo_t *xdo, int dest_x, int dest_y);

/**
 * Send a click for a specific mouse button at the current mouse location.
 *
 * @param window The window you want to send the event to or CURRENTWINDOW
 * @param button The mouse button. Generally, 1 is left, 2 is middle, 3 is
 *    right, 4 is wheel up, 5 is wheel down.
 */
int xdo_click(const xdo_t *xdo, Window window, int button);

/**
 * Send a one or more clicks for a specific mouse button at the current mouse
 * location.
 *
 * @param window The window you want to send the event to or CURRENTWINDOW
 * @param button The mouse button. Generally, 1 is left, 2 is middle, 3 is
 *    right, 4 is wheel up, 5 is wheel down.
 */
int xdo_click_multiple(const xdo_t *xdo, Window window, int button,
                       int repeat, useconds_t delay);

/**
 * Type a string to the specified window.
 *
 * If you want to send a specific key or key sequence, such as "alt+l", you
 * want instead xdo_keysequence(...).
 *
 * @param window The window you want to send keystrokes to or CURRENTWINDOW
 * @param string The string to type, like "Hello world!"
 * @param delay The delay between keystrokes in microseconds. 12000 is a decent
 *    choice if you don't have other plans.
 */
int xdo_type(const xdo_t *xdo, Window window, char *string, useconds_t delay);

/**
 * Send a keysequence to the specified window.
 *
 * This allows you to send keysequences by symbol name. Any combination
 * of X11 KeySym names separated by '+' are valid. Single KeySym names
 * are valid, too.
 *
 * Examples:
 *   "l"
 *   "semicolon"
 *   "alt+Return"
 *   "Alt_L+Tab"
 *
 * If you want to type a string, such as "Hello world." you want to instead
 * use xdo_type.
 *
 * @param window The window you want to send the keysequence to or
 *   CURRENTWINDOW
 * @param keysequence The string keysequence to send.
 * @param delay The delay between keystrokes in microseconds.
 */
int xdo_keysequence(const xdo_t *xdo, Window window,
                    const char *keysequence, useconds_t delay);

/**
 * Send key release (up) events for the given key sequence.
 *
 * @see xdo_keysequence
 */
int xdo_keysequence_up(const xdo_t *xdo, Window window,
                       const char *keysequence, useconds_t delay);

/**
 * Send key press (down) events for the given key sequence.
 *
 * @see xdo_keysequence
 */
int xdo_keysequence_down(const xdo_t *xdo, Window window,
                         const char *keysequence, useconds_t delay);
                         
/**
 * Send a series of keystrokes.
 *
 * @param window The window to send events to or CURRENTWINDOW
 * @param keys The array of charcodemap_t entities to send.
 * @param nkeys The length of the keys parameter
 * @param pressed 1 for key press, 0 for key release.
 * @param modifier Pointer to integer to record the modifiers activated by
 *   the keys being pressed. If NULL, we don't save the modifiers.
 * @param delay The delay between keystrokes in microseconds.
 */
int xdo_keysequence_list_do(const xdo_t *xdo, Window window,
                            charcodemap_t *keys, int nkeys,
                            int pressed, int *modifier, useconds_t delay);

/**
 * Get a list of active keys. Uses XQueryKeymap.
 *
 * @param keys Pointer to the array of charcodemap_t that will be allocated
 *    by this function.
 * @param nkeys Pointer to integer where the number of keys will be stored.
 */
int xdo_active_keys_to_keycode_list(const xdo_t *xdo, charcodemap_t **keys,
                                         int *nkeys);

/**
 * Wait for a window to have a specific map state.
 *
 * State possibilities:
 *   IsUnmapped - window is not displayed.
 *   IsViewable - window is mapped and shown (though may be clipped by windows
 *     on top of it)
 *   IsUnviewable - window is mapped but a parent window is unmapped.
 *
 * @param wid the window you want to wait for.
 * @param map_state the state to wait for.
 */
int xdo_window_wait_for_map_state(const xdo_t *xdo, Window wid, int map_state);

#define SIZE_TO 0
#define SIZE_FROM 1
int xdo_window_wait_for_size(const xdo_t *xdo, Window window, unsigned int width,
                             unsigned int height, int flags, int to_or_from);


/**
 * Move a window to a specific location.
 *
 * The top left corner of the window will be moved to the x,y coordinate.
 *
 * @param wid the window to move
 * @param x the X coordinate to move to.
 * @param y the Y coordinate to move to.
 */
int xdo_window_move(const xdo_t *xdo, Window wid, int x, int y);

/**
 * Apply a window's sizing hints (if any) to a given width and height.
 *
 * This function wraps XGetWMNormalHints() and applies any 
 * resize increment and base size to your given width and height values.
 *
 * @param window the window to use
 * @param width the unit width you want to translate
 * @param height the unit height you want to translate
 * @param width_ret the return location of the translated width
 * @param height_ret the return locatino of the translated height
 */
int xdo_window_translate_with_sizehint(const xdo_t *xdo, Window window,
                                       int width, int height, int *width_ret,
                                       int *height_ret);

/**
 * Change the window size.
 *
 * @param wid the window to resize
 * @param w the new desired width
 * @param h the new desired height
 * @param flags if 0, use pixels for units. If SIZE_USEHINTS, then
 *   the units will be relative to the window size hints.
 */
int xdo_window_setsize(const xdo_t *xdo, Window wid, int w, int h, int flags);

/**
 * Change a window property.
 *
 * Example properties you can change are WM_NAME, WM_ICON_NAME, etc.
 *
 * @param wid The window to change a property of.
 * @param property the string name of the property.
 * @param value the string value of the property.
 */
int xdo_window_setprop (const xdo_t *xdo, Window wid, const char *property,
                        const char *value);

/**
 * Change the window's classname and or class.
 *
 * @param name The new class name. If NULL, no change.
 * @param class The new class. If NULL, no change.
 */
int xdo_window_setclass(const xdo_t *xdo, Window wid, const char *name,
                        const char *class);

/**
 * Sets the urgency hint for a window.
 */
int xdo_window_seturgency (const xdo_t *xdo, Window wid, int urgency);

/**
 * Set the override_redirect value for a window. This generally means
 * whether or not a window manager will manage this window.
 *
 * If you set it to 1, the window manager will usually not draw borders on the
 * window, etc. If you set it to 0, the window manager will see it like a
 * normal application window.
 *
 */
int xdo_window_set_override_redirect(const xdo_t *xdo, Window wid,
                                     int override_redirect);

/**
 * Focus a window.
 *
 * @see xdo_window_activate
 * @param wid the window to focus.
 */
int xdo_window_focus(const xdo_t *xdo, Window wid);

/**
 * Raise a window to the top of the window stack. This is also sometimes
 * termed as bringing the window forward.
 *
 * @param wid The window to raise.
 */
int xdo_window_raise(const xdo_t *xdo, Window wid);

/**
 * Get the window currently having focus.
 *
 * @param window_ret Pointer to a window where the currently-focused window
 *   will be stored.
 */
int xdo_window_get_focus(const xdo_t *xdo, Window *window_ret);

/**
 * Wait for a window to have or lose focus.
 *
 * @param window The window to wait on
 * @param want_focus If 1, wait for focus. If 0, wait for loss of focus.
 */
int xdo_window_wait_for_focus(const xdo_t *xdo, Window window, int want_focus);

/**
 * Get the PID owning a window. Not all applications support this.
 * It looks at the _NET_WM_PID property of the window.
 *
 * @param window the window to query.
 * @return the process id or 0 if no pid found.
 */
int xdo_window_get_pid(const xdo_t *xdo, Window window);

/**
 * Like xdo_window_get_focus, but return the first ancestor-or-self window *
 * having a property of WM_CLASS. This allows you to get the "real" or
 * top-level-ish window having focus rather than something you may not expect
 * to be the window having focused.
 *
 * @param window_ret Pointer to a window where the currently-focused window
 *   will be stored.
 */
int xdo_window_sane_get_focus(const xdo_t *xdo, Window *window_ret);

/**
 * Activate a window. This is generally a better choice than xdo_window_focus
 * for a variety of reasons, but it requires window manager support:
 *   - If the window is on another desktop, that desktop is switched to.
 *   - It moves the window forward rather than simply focusing it
 *
 * Requires your window manager to support this.
 * Uses _NET_ACTIVE_WINDOW from the EWMH spec.
 *
 * @param wid the window to activate
 */
int xdo_window_activate(const xdo_t *xdo, Window wid);

/**
 * Wait for a window to be active or not active.
 *
 * Requires your window manager to support this.
 * Uses _NET_ACTIVE_WINDOW from the EWMH spec.
 *
 * @param window the window to wait on
 * @param active If 1, wait for active. If 0, wait for inactive.
 */
int xdo_window_wait_for_active(const xdo_t *xdo, Window window, int active);

/**
 * Map a window. This mostly means to make the window visible if it is
 * not currently mapped.
 *
 * @param wid the window to map.
 */
int xdo_window_map(const xdo_t *xdo, Window wid);

/**
 * Unmap a window
 *
 * @param wid the window to unmap
 */
int xdo_window_unmap(const xdo_t *xdo, Window wid);

/**
 * Minimize a window.
 */
int xdo_window_minimize(const xdo_t *xdo, Window wid);

/** 
 * Reparents a window
 *
 * @param wid_source the window to reparent
 * @param wid_target the new parent window
 */
int xdo_window_reparent(const xdo_t *xdo, Window wid_source, Window wid_target);

/**
 * Get a window's location.
 *
 * @param wid the window to query
 * @param x_ret pointer to int where the X location is stored. If NULL, X is
 *   ignored.
 * @param y_ret pointer to int where the Y location is stored. If NULL, X is
 *   ignored.
 * @param screen_ret Pointer to Screen* where the Screen* the window on is
 *   stored. If NULL, this parameter is ignored.
 */
int xdo_get_window_location(const xdo_t *xdo, Window wid,
                            int *x_ret, int *y_ret, Screen **screen_ret);

/**
 * Get a window's size.
 *
 * @param wid the window to query
 * @param width_ret pointer to unsigned int where the width is stored.
 * @param height_ret pointer to unsigned int where the height is stored.
 */
int xdo_get_window_size(const xdo_t *xdo, Window wid, unsigned int *width_ret,
                        unsigned int *height_ret);

/* pager-like behaviors */

/**
 * Get the currently-active window.
 * Requires your window manager to support this.
 * Uses _NET_ACTIVE_WINDOW from the EWMH spec.
 *
 * @param window_ret Pointer to Window where the active window is stored.
 */
int xdo_window_get_active(const xdo_t *xdo, Window *window_ret);

/**
 * Get a window ID by clicking on it. This function blocks until a selection
 * is made.
 *
 * @param window_ret Pointer to Window where the selected window is stored.
 */
int xdo_window_select_with_click(const xdo_t *xdo, Window *window_ret);

/**
 * Set the number of desktops.
 * Uses _NET_NUMBER_OF_DESKTOPS of the EWMH spec.
 *
 * @param ndesktops the new number of desktops to set.
 */
int xdo_set_number_of_desktops(const xdo_t *xdo, long ndesktops);

/**
 * Get the current number of desktops.
 * Uses _NET_NUMBER_OF_DESKTOPS of the EWMH spec.
 *
 * @param ndesktops pointer to long where the current number of desktops is
 *   stored
 */
int xdo_get_number_of_desktops(const xdo_t *xdo, long *ndesktops);

/**
 * Switch to another desktop.
 * Uses _NET_CURRENT_DESKTOP of the EWMH spec.
 *
 * @param desktop The desktop number to switch to.
 */
int xdo_set_current_desktop(const xdo_t *xdo, long desktop);

/**
 * Get the current desktop.
 * Uses _NET_CURRENT_DESKTOP of the EWMH spec.
 *
 * @param desktop pointer to long where the current desktop number is stored.
 */
int xdo_get_current_desktop(const xdo_t *xdo, long *desktop);

/**
 * Move a window to another desktop
 * Uses _NET_WM_DESKTOP of the EWMH spec.
 *
 * @param wid the window to move
 * @param desktop the desktop destination for the window
 */
int xdo_set_desktop_for_window(const xdo_t *xdo, Window wid, long desktop);

/**
 * Get the desktop a window is on.
 * Uses _NET_WM_DESKTOP of the EWMH spec.
 *
 * @param wid the window to query
 * @param deskto pointer to long where the desktop of the window is stored
 */
int xdo_get_desktop_for_window(const xdo_t *xdo, Window wid, long *desktop);

/**
 * Search for windows.
 *
 * @param search the search query.
 * @param windowlist_ret the list of matching windows to return
 * @param nwindows_ret the number of windows (length of windowlist_ret)
 * @see xdo_search_t
 */
int xdo_window_search(const xdo_t *xdo, const xdo_search_t *search,
                      Window **windowlist_ret, int *nwindows_ret);

/**
 * Generic property fetch.
 *
 * @param window the window to query
 * @param atom the Atom to request
 * @param nitems the number of items 
 * @param type the type of the return
 * @param size the size of the type
 * @return data consisting of 'nitems' items of size 'size' and type 'type'
 *   will need to be cast to the type before using.
 */
unsigned char *xdo_getwinprop(const xdo_t *xdo, Window window, Atom atom,
                              long *nitems, Atom *type, int *size);

/**
 * Get the current input state. This is a mask value containing any of the
 * following: ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
 * Mod4Mask, or Mod5Mask.
 *
 * @return the input mask
 */
unsigned int xdo_get_input_state(const xdo_t *xdo);

/**
 * If you need the keysym-to-character map, you can fetch it using this method.
 * @see keysym_charmap_t
 */
const keysym_charmap_t *xdo_keysym_charmap(void);

/**
 * If you need the symbol map, use this method.
 *
 * The symbol map is an array of string pairs mapping common tokens to X Keysym
 * strings, such as "alt" to "Alt_L"
 *
 * @returns array of strings.
 */
const char **xdo_symbol_map(void);

/* active modifiers stuff */

/**
 * Get the currently-active modifiers
 */
xdo_active_mods_t *xdo_get_active_modifiers(const xdo_t *xdo);

/**
 * Send any events necesary to clear the the active modifiers.
 * For example, if you are holding 'alt' when xdo_get_active_modifiers is 
 * called, then this method will send a key-up for 'alt'
 */
int xdo_clear_active_modifiers(const xdo_t *xdo, Window window,
                               xdo_active_mods_t *active_mods);

/**
 * Send any events necessary to make these modifiers active.
 * This is useful if you just cleared the active modifiers and then wish
 * to restore them after.
 */
int xdo_set_active_modifiers(const xdo_t *xdo, Window window,
                             const xdo_active_mods_t *active_mods);

/**
 * Free the data allocated by xdo_get_active_modifiers.
 */
void xdo_free_active_modifiers(xdo_active_mods_t *active_mods);

/**
 * Get the position of the current viewport.
 *
 * This is only relevant if your window manager supports
 * _NET_DESKTOP_VIEWPORT 
 */
int xdo_get_desktop_viewport(const xdo_t *xdo, int *x_ret, int *y_ret);

/**
 * Set the position of the current viewport.
 *
 * This is only relevant if your window manager supports
 * _NET_DESKTOP_VIEWPORT
 */
int xdo_set_desktop_viewport(const xdo_t *xdo, int x, int y);

/**
 * Kill a window and the client owning it.
 *
 */
int xdo_window_kill(const xdo_t *xdo, Window window);

/**
 * Find a client window that is a parent of the window given
 */
#define XDO_FIND_PARENTS (0)

/**
 * Find a client window that is a child of the window given
 */
#define XDO_FIND_CHILDREN (1)

/**
 * Find a client window (child) in a given window. Useful if you get the
 * window manager's decorator window rather than the client window.
 */
int xdo_window_find_client(const xdo_t *xdo, Window window, Window *window_ret,
                           int direction);

/**
 * Get a window's name, if any.
 *
 * TODO(sissel): Document
 */
int xdo_get_window_name(const xdo_t *xdo, Window window, 
                        unsigned char **name_ret, int *name_len_ret,
                        int *name_type);

#endif /* ifndef _XDO_H_ */
