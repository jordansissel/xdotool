#ifndef _XDOTOOL_H_
#define _XDOTOOL_H_

/* TODO(sissel): use proper printf format depending on the storage
 * size of Window (could be 4 or 8 bytes depending on platform */
#define window_print(window) (printf("%ld\n", window))
#define window_each(context, window_arg, block) \
{ \
  Window *windows; \
  int nwindows; \
  window_list(context, window_arg, &windows, &nwindows, False); \
  int w_index;\
  for (w_index = 0; w_index < nwindows; w_index++) { \
    Window window = windows[w_index]; \
    {  \
      block \
    } \
  } \
} /* end define window_each */

  

typedef struct context {
  xdo_t *xdo;
  const char *prog;
  int argc;
  char **argv;
  int debug;

  /* Window stack */
  Window *windows;
  int nwindows;
  Window window_placeholder[1];

  /* Last known mouse position */
  int last_mouse_x;
  int last_mouse_y;
  int last_mouse_screen;
  int have_last_mouse;
} context_t;

int xdotool_main(int argc, char **argv);
int cmd_exec(context_t *context);
int cmd_sleep(context_t *context);
int cmd_behave(context_t *context);
int cmd_behave_screen_edge(context_t *context);
int cmd_click(context_t *context);
int cmd_getactivewindow(context_t *context);
int cmd_getmouselocation(context_t *context);
int cmd_getwindowfocus(context_t *context);
int cmd_getwindowname(context_t *context);
int cmd_getwindowclassname(context_t *context);
int cmd_getwindowpid(context_t *context);
int cmd_getwindowgeometry(context_t *context);
int cmd_help(context_t *context);
int cmd_key(context_t *context);
int cmd_mousedown(context_t *context);
int cmd_mousemove(context_t *context);
int cmd_mousemove_relative(context_t *context);
int cmd_mouseup(context_t *context);
int cmd_search(context_t *context);
int cmd_set_window(context_t *context);
int cmd_type(context_t *context);
int cmd_version(context_t *context);
int cmd_window_select(context_t *context);
int cmd_windowactivate(context_t *context);
int cmd_windowfocus(context_t *context);
int cmd_windowkill(context_t *context);
int cmd_windowclose(context_t *context);
int cmd_windowquit(context_t *context);
int cmd_windowmap(context_t *context);
int cmd_windowminimize(context_t *context);
int cmd_windowmove(context_t *context);
int cmd_windowraise(context_t *context);
int cmd_windowreparent(context_t *context);
int cmd_windowsize(context_t *context);
int cmd_windowstate(context_t *context);
int cmd_windowunmap(context_t *context);
/* pager-like commands */
int cmd_set_num_desktops(context_t *context);
int cmd_get_num_desktops(context_t *context);
int cmd_set_desktop(context_t *context);
int cmd_get_desktop(context_t *context);
int cmd_set_desktop_for_window(context_t *context);
int cmd_get_desktop_for_window(context_t *context);
int cmd_set_desktop_viewport(context_t *context);
int cmd_get_desktop_viewport(context_t *context);
int cmd_get_display_geometry(context_t *context);

#endif /* _XDOTOOL_H_ */
