/* xdotool
 *
 * command line interface to the xdo library
 *
 * $Id$
 *
 * getwindowfocus contributed by Lee Pumphret
 * keyup/down contributed by Lee Pumphret
 *
 * XXX: Need to use 'Window' instead of 'int' where appropriate.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>


#include "xdo.h"

int cmd_click(int argc, char **args);
int cmd_getwindowfocus(int argc, char **args);
int cmd_help(int argc, char **args);
int cmd_key(int argc, char **args);
int cmd_mousedown(int argc, char **args);
int cmd_mousemove(int argc, char **args);
int cmd_mousemove_relative(int argc, char **args);
int cmd_mouseup(int argc, char **args);
int cmd_search(int argc, char **args);
int cmd_type(int argc, char **args);
int cmd_windowactivate(int argc, char **args);
int cmd_windowfocus(int argc, char **args);
int cmd_windowmap(int argc, char **args);
int cmd_windowmove(int argc, char **args);
int cmd_windowraise(int argc, char **args);
int cmd_windowsize(int argc, char **args);
int cmd_windowunmap(int argc, char **args);

/* pager-like commands */
int cmd_set_num_desktops(int argc, char **args);
int cmd_get_num_desktops(int argc, char **args);
int cmd_set_desktop(int argc, char **args);
int cmd_get_desktop(int argc, char **args);
int cmd_set_desktop_for_window(int argc, char **args);
int cmd_get_desktop_for_window(int argc, char **args);

xdo_t *xdo;
void window_print(Window wid);

struct dispatch {
  const char *name;
  int (*func)(int argc, char **args);
} dispatch[] = {
  /* Query functions */
  { "getwindowfocus", cmd_getwindowfocus, },
  { "search", cmd_search, },

  /* Help me! */
  { "help", cmd_help, },
  { "-h", cmd_help, },

  /* Action functions */
  { "click", cmd_click, },
  { "key", cmd_key, },
  { "keydown", cmd_key, },
  { "keyup", cmd_key, },
  { "mousedown", cmd_mousedown, },
  { "mousemove", cmd_mousemove, },
  { "mousemove_relative", cmd_mousemove_relative, },
  { "mouseup", cmd_mouseup, },
  { "type", cmd_type, },
  { "windowactivate", cmd_windowactivate, },
  { "windowfocus", cmd_windowfocus, },
  { "windowmap", cmd_windowmap, },
  { "windowmove", cmd_windowmove, },
  { "windowraise", cmd_windowraise, },
  { "windowsize", cmd_windowsize, },
  { "windowunmap", cmd_windowunmap, },

  { "set_num_desktops", cmd_set_num_desktops, },
  { "get_num_desktops", cmd_get_num_desktops, },
  { "set_desktop", cmd_set_desktop, },
  { "get_desktop", cmd_get_desktop, },
  { "set_desktop_for_window", cmd_set_desktop_for_window, },
  { "get_desktop_for_window", cmd_get_desktop_for_window, },
  { NULL, NULL, },
};

int main(int argc, char **argv) {
  char *cmd;
  char *prog;
  int ret = 0;
  int cmd_found = 0;
  int i;

  if (argc < 2) {
    fprintf(stderr, "usage: %s <cmd> <args>\n", argv[0]);
    cmd_help(0, NULL);
    exit(1);
  }

  prog = *argv;
  argv++; argc--;
  cmd = *argv; /* argv[1] */

  xdo = xdo_new(getenv("DISPLAY"));
  if (xdo == NULL) {
    fprintf(stderr, "Failed creating new xdo instance\n");
    return 1;
  }

  for (i = 0; dispatch[i].name != NULL && !cmd_found; i++) {
    if (!strcasecmp(dispatch[i].name, cmd)) {
      ret = dispatch[i].func(argc, argv);
      cmd_found = 1;
    }
  }

  if (!cmd_found) {
    fprintf(stderr, "Unknown command: %s\n", cmd);
    fprintf(stderr, "Run '%s help' if you want a command list\n", prog);
    ret = 1;
  }

  xdo_free(xdo);
  return ret;
}

void window_print(Window wid) {
  /* Window is XID is 'unsigned long' or CARD32 */
  printf("%ld\n", wid);
}

int cmd_help(int argc, char **args) {
  int i;
  printf("Available commands:\n");
  for (i = 0; dispatch[i].name != NULL; i++)
    printf("  %s\n", dispatch[i].name);

  return 0;
}

int cmd_mousemove(int argc, char **args) {
  int ret = 0;
  int x, y;
  char *cmd = *args; argc--; args++;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <xcoord> <ycoord>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  x = atoi(args[0]);
  y = atoi(args[1]);

  ret = xdo_mousemove(xdo, x, y);

  if (ret)
    fprintf(stderr, "xdo_mousemove reported an error\n");

  return ret;
}

int cmd_mousemove_relative(int argc, char **args) {
  int x, y;
  int ret = 0;
  char *cmd = *args; argc--; args++;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <xcoord> <ycoord>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  x = atoi(args[0]);
  y = atoi(args[1]);

  ret = xdo_mousemove_relative(xdo, x, y);

  if (ret)
    fprintf(stderr, "xdo_mousemove_relative reported an error\n");

  return ret;
}

int cmd_mousedown(int argc, char **args) {
  int ret = 0;
  int button;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    fprintf(stderr, "usage: %s <button>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(args[0]);

  ret = xdo_mousedown(xdo, button);

  if (ret)
    fprintf(stderr, "xdo_mousedown reported an error\n");

  return ret;
}

int cmd_mouseup(int argc, char **args) {
  int ret = 0;
  int button;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    fprintf(stderr, "usage: %s <button>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(args[0]);

  ret = xdo_mouseup(xdo, button);
  if (ret)
    fprintf(stderr, "xdo_mouseup reported an error\n");
  
  return ret;
}

int cmd_click(int argc, char **args) {
  int button;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    fprintf(stderr, "usage: %s <button>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(args[0]);
  return xdo_click(xdo, button);
}

int cmd_type(int argc, char **args) {
  int ret = 0;
  int i;
  char *cmd = *args; argc--; args++;

  if (argc == 0) {
    fprintf(stderr, "usage: %s <things to type>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  for (i = 0; i < argc; i++) {
    int tmp = xdo_type(xdo, args[i]);

    if (tmp) {
      fprintf(stderr, "xdo_type reported an error\n");
    }

    ret += tmp;
  }

  return ret > 0;
}

int cmd_key(int argc, char **args) {
  int ret = 0;
  int i;
  char *cmd = *args; argc--; args++;

  if (argc == 0) {
    fprintf(stderr, "usage: %s <keyseq1> [keyseq2 ... keyseqN]\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  int (*func)(xdo_t *, char *) = NULL;

  if (!strcmp(cmd, "key")) {
    func = xdo_keysequence;
  } else if (!strcmp(cmd, "keyup")) {
    func = xdo_keysequence_up;
  } else if (!strcmp(cmd, "keydown")) {
    func = xdo_keysequence_down;
  } else {
    fprintf(stderr, "Unknown command '%s'\n", cmd);
    return 1;
  }

  for (i = 0; i < argc; i++) {
    int tmp = func(xdo, args[i]);
    if (tmp != 0)
      fprintf(stderr, "xdo_keysequence reported an error for string '%s'\n", args[i]);
    ret += tmp;
  }

  return ret;
}

int cmd_windowmove(int argc, char **args) {
  int ret = 0;
  int x, y;
  Window wid;
  char *cmd = *args; argc--; args++;

  if (argc != 3) {
    printf("usage: %s wid x y\n", cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  x = (int)strtol(args[1], NULL, 0);
  y = (int)strtol(args[2], NULL, 0);

  ret = xdo_window_move(xdo, wid, x, y);
  if (ret)
    fprintf(stderr, "xdo_window_move reported an error\n");
  
  return ret;
}

int cmd_windowactivate(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_activate(xdo, wid);

  if (ret)
    fprintf(stderr, "xdo_window_activate reported an error\n");

  return ret;
}

int cmd_windowfocus(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_focus(xdo, wid);
  if (ret)
    fprintf(stderr, "xdo_window_focus reported an error\n");
  
  return ret;
}

int cmd_windowraise(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return 1;
   }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_raise(xdo, wid);
  if (ret)
    fprintf(stderr, "xdo_window_raise reported an error\n");
  
  return ret;
}

int cmd_windowsize(int argc, char **args) {
  int ret = 0;
  int width, height;
  Window wid;
  int c;

  int use_hints = 0;
  struct option longopts[] = {
    { "usehints", 0, &use_hints, 1 },
    { 0, 0, 0, 0 },
  };

  int size_flags = 0;
  char *cmd = *args;

  while (1) {
    int option_index;

    c = getopt_long_only(argc, args, "", longopts, &option_index);

    if (c == -1)
      break;
  }

  if (use_hints)
    size_flags |= SIZE_USEHINTS;

  args += optind;
  argc -= optind;

  if (argc != 3) {
    printf("usage: %s wid width height\n", cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  width = (int)strtol(args[1], NULL, 0);
  height = (int)strtol(args[2], NULL, 0);

  ret = xdo_window_setsize(xdo, wid, width, height, size_flags);
  if (ret)
    fprintf(stderr, "xdo_window_setsize reported an error\n");
  return ret;
}

int cmd_search(int argc, char **args) {
  Window *list;
  int nwindows;
  int i;
  int c;

  int only_visible = 0;
  int search_title = 0;
  int search_name = 0;
  int search_class = 0;
  struct option longopts[] = {
    { "onlyvisible", 0, &only_visible, 1 },
    { "title", 0, &search_title, 1 },
    { "name", 0, &search_name, 1 },
    { "class", 0, &search_class, 1 },
    { 0, 0, 0, 0 },
  };

  int search_flags = 0;

  char *cmd = *args;

  while (1) {
    int option_index;

    c = getopt_long_only(argc, args, "", longopts, &option_index);

    if (c == -1)
      break;
  }

  if (only_visible)
    search_flags |= SEARCH_VISIBLEONLY;
  if (search_title)
    search_flags |= SEARCH_TITLE;
  if (search_name)
    search_flags |= SEARCH_NAME;
  if (search_class)
    search_flags |= SEARCH_CLASS;

  args += optind;
  argc -= optind;

  if (argc != 1) {
    printf(
      "Usage: xdotool %s "
      "[--onlyvisible] [--title --class --name] regexp_pattern\n"
      " --onlyvisible   matches only windows currently visible\n"
      " --title         check regexp_pattern agains the window title\n"
      " --class         check regexp_pattern agains the window class\n"
      " --name          check regexp_pattern agains the window name\n"
      "\n"
      "* If none of --title, --class, and --name are specified,\n"
      "the defaults are to match any of them.\n", 
      cmd);
    return 1;
  }

  xdo_window_list_by_regex(xdo, *args, search_flags, &list, &nwindows);
  for (i = 0; i < nwindows; i++)
    window_print(list[i]);

  /* Free list as it's malloc'd by xdo_window_list_by_regex */
  free(list);

  /* error if number of windows found is zero (behave like grep) */
  return !nwindows;
}

/* Added 2007-07-28 - Lee Pumphret */
int cmd_getwindowfocus(int argc, char **args) {
  int ret = 0;
  Window wid = 0;
  char *cmd = *args; argc--; args++;

  if (argc != 0) {
    printf("usage: %s\n", cmd);
    return 1;
  }

  ret = xdo_window_get_focus(xdo, &wid);

  if (ret) {
    fprintf(stderr, "xdo_window_focus reported an error\n");
  } else { 
    window_print(wid);
  }

  return ret;
}

int cmd_windowmap(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_map(xdo, wid);
  if (ret)
    fprintf(stderr, "xdo_window_map reported an error\n");
  
  return ret;
}

int cmd_windowunmap(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args; argc--; args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_unmap(xdo, wid);
  if (ret)
    fprintf(stderr, "xdo_window_unmap reported an error\n");
  
  return ret;
}

int cmd_set_num_desktops(int argc, char **args) {
  char *cmd = *args; argc--; args++;
  long ndesktops;

  if (argc != 1) {
    printf("usage: %s num_desktops\n", cmd);
    return 1;
  }

  ndesktops = strtol(args[0], NULL, 0);

  return xdo_set_number_of_desktops(xdo, ndesktops);
}

int cmd_get_num_desktops(int argc, char **args) {
  int ret = 0;
  char *cmd = *args; argc--; args++;
  long ndesktops = 0;

  if (argc != 0) {
    printf("usage: %s\n", cmd);
    return 1;
  }

  ret = xdo_get_number_of_desktops(xdo, &ndesktops);

  printf("%ld\n", ndesktops);
  return ret;
}

int cmd_set_desktop(int argc, char **args) {
  char *cmd = *args; argc--; args++;
  long desktop;

  if (argc != 1) {
    printf("usage: %s desktop\n", cmd);
    return 1;
  }

  desktop = strtol(args[0], NULL, 0);

  return xdo_set_current_desktop(xdo, desktop);
}

int cmd_get_desktop(int argc, char **args) {
  int ret = 0;
  char *cmd = *args; argc--; args++;
  long desktop = 0;

  if (argc != 0) {
    printf("usage: %s\n", cmd);
    return 1;
  }

  ret = xdo_get_current_desktop(xdo, &desktop);
  printf("%ld\n", desktop);
  return ret;
}

int cmd_set_desktop_for_window(int argc, char **args) {
  char *cmd = *args; argc--; args++;
  long desktop = 0;
  Window window = 0;

  if (argc != 2) {
    printf("usage: %s <window> <desktop>\n", cmd);
    return 1;
  }

  window = (Window)strtol(args[0], NULL, 0);
  desktop = strtol(args[1], NULL, 0);

  return xdo_set_desktop_for_window(xdo, window, desktop);
}

int cmd_get_desktop_for_window(int argc, char **args) {
  int ret = 0;
  char *cmd = *args; argc--; args++;
  long desktop = 0;
  Window window = 0;

  if (argc != 1) {
    printf("usage: %s <window>\n", cmd);
    return 1;
  }

  window = (Window)strtol(args[0], NULL, 0);

  ret = xdo_get_desktop_for_window(xdo, window, &desktop);
  printf("%ld\n", desktop);
  return ret;
}
