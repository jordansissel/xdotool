#ifndef _XDOTOOL_H_
#define _XDOTOOL_H_

/* TODO(sissel): use proper printf format depending on the storage
 * size of Window (could be 4 or 8 bytes depending on platform */
#define window_print(window) (printf("%ld\n", window))

int cmd_click(int argc, char **args);
int cmd_getwindowfocus(int argc, char **args);
int cmd_getwindowpid(int argc, char **args);
int cmd_getactivewindow(int argc, char **args);
int cmd_help(int argc, char **args);
int cmd_key(int argc, char **args);
int cmd_mousedown(int argc, char **args);
int cmd_mousemove(int argc, char **args);
int cmd_mousemove_relative(int argc, char **args);
int cmd_mouseup(int argc, char **args);
int cmd_getmouselocation(int argc, char **args);
int cmd_search(int argc, char **args);
int cmd_type(int argc, char **args);
int cmd_windowactivate(int argc, char **args);
int cmd_windowfocus(int argc, char **args);
int cmd_windowmap(int argc, char **args);
int cmd_windowmove(int argc, char **args);
int cmd_windowraise(int argc, char **args);
int cmd_windowsize(int argc, char **args);
int cmd_windowunmap(int argc, char **args);
int cmd_set_window(int argc, char** args);
int cmd_version(int argc, char** args);
  
/* pager-like commands */
int cmd_set_num_desktops(int argc, char **args);
int cmd_get_num_desktops(int argc, char **args);
int cmd_set_desktop(int argc, char **args);
int cmd_get_desktop(int argc, char **args);
int cmd_set_desktop_for_window(int argc, char **args);
int cmd_get_desktop_for_window(int argc, char **args);

#endif /* _XDOTOOL_H_ */
