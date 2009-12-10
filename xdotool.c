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
#define _GNU_SOURCE 1
#define __USE_BSD /* for strdup on linux/glibc */
#include <getopt.h>
#include <string.h>
#include <strings.h>


#include "xdo.h"

int cmd_click(int argc, char **args);
int cmd_getwindowfocus(int argc, char **args);
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
  { "getactivewindow", cmd_getactivewindow, },
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
  { "getmouselocation", cmd_getmouselocation, },
  { "type", cmd_type, },
  { "windowactivate", cmd_windowactivate, },
  { "windowfocus", cmd_windowfocus, },
  { "windowmap", cmd_windowmap, },
  { "windowmove", cmd_windowmove, },
  { "windowraise", cmd_windowraise, },
  { "windowsize", cmd_windowsize, },
  { "windowunmap", cmd_windowunmap, },

  { "set_window", cmd_set_window, },

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
  int opt;
  int option_index;
  const char *usage = "Usage: %s <cmd> <args>\n";
  static struct option long_options[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) {
    fprintf(stderr, usage, argv[0]);
    cmd_help(0, NULL);
    exit(1);
  }

  while ((opt = getopt_long_only(argc, argv, "+h", long_options, &option_index)) != -1) {
    switch (opt) {
      case 'h':
        cmd_help(0, NULL);
        exit(EXIT_SUCCESS);
      default:
        fprintf(stderr, usage, argv[0]);
        exit(EXIT_FAILURE);
    }
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

int cmd_help(int unused_argc, char **unused_args) {
  int i;
  printf("Available commands:\n");
  for (i = 0; dispatch[i].name != NULL; i++)
    printf("  %s\n", dispatch[i].name);

  return 0;
}

int cmd_mousemove(int argc, char **args) {
  int ret = 0;
  int x, y;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <xcoord> <ycoord>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 2) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <xcoord> <ycoord>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 2) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <button>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <button>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(args[0]);

  ret = xdo_mouseup(xdo, button);
  if (ret)
    fprintf(stderr, "xdo_mouseup reported an error\n");

  return ret;
}

int cmd_getmouselocation(int argc, char **args) {
  int x, y, screen_num;
  int ret;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 0) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ret = xdo_mouselocation(xdo, &x, &y, &screen_num);
  printf("x:%d y:%d screen:%d\n", x, y, screen_num);
  return ret;
}

int cmd_click(int argc, char **args) {
  int button;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <button>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(args[0]);
  return xdo_click(xdo, button);
}

int cmd_type(int argc, char **args) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *args;
  charcodemap_t *keymods;
  int nkeymods;

  /* Options */
  int clear_modifiers = 0;
  Window window = 0;
  useconds_t delay = 12000; /* 12ms between keystrokes default */

  struct option longopts[] = {
    { "window", required_argument, NULL, 'w' },
    { "delay", required_argument, NULL, 'd' },
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };

  static const char *usage =
            "Usage: %s [--window windowid] [--delay milliseconds] "
            "<things to type>\n"
            "--window <windowid>    - specify a window to send keys to\n"
            "--delay <milliseconds> - delay between keystrokes\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
            "-h, --help             - show this help output\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "w:d:ch", longopts, &option_index)) != -1) {
    switch (c) {
      case 'w':
        window = strtoul(optarg, NULL, 0);
        break;
      case 'd':
        /* --delay is in milliseconds, convert to microseconds */
        delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  args += optind;
  argc -= optind;

  if (argc == 0) {
    fprintf(stderr, "You specified the wrong number of args.\n");
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (clear_modifiers) {
    xdo_active_modifiers_to_keycode_list(xdo, &keymods, &nkeymods);
    xdo_keysequence_list_do(xdo, window, keymods, nkeymods, False, NULL);
  }

  for (i = 0; i < argc; i++) {
    int tmp = xdo_type(xdo, window, args[i], delay);

    if (tmp) {
      fprintf(stderr, "xdo_type reported an error\n");
    }

    ret += tmp;
  }

  if (clear_modifiers) {
    /* Re-activate any modifiers we disabled previously */
    xdo_keysequence_list_do(xdo, window, keymods, nkeymods, True, NULL);
    free(keymods);
  }

  return ret > 0;
}

int cmd_key(int argc, char **args) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *args;
  charcodemap_t *keymods;
  int nkeymods;

  /* Options */
  Window window = 0;
  int clear_modifiers = 0;

  static struct option longopts[] = {
    { "window", required_argument, NULL, 'w' },
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };

  static const char *usage = "Usage: %s [--window windowid] [--clearmodifiers] <keyseq1> [keyseq2 ... keyseqN]\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "hw:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'w':
        window = strtoul(optarg, NULL, 0);
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc == 0) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  int (*func)(xdo_t *, Window, char *) = NULL;

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

  if (clear_modifiers) {
    xdo_active_modifiers_to_keycode_list(xdo, &keymods, &nkeymods);
    xdo_keysequence_list_do(xdo, window, keymods, nkeymods, False, NULL);
  }

  for (i = 0; i < argc; i++) {
    int tmp = func(xdo, window, args[i]);
    if (tmp != 0)
      fprintf(stderr, "xdo_keysequence reported an error for string '%s'\n", args[i]);
    ret += tmp;
  }

  if (clear_modifiers) {
    /* Re-activate any modifiers we disabled previously */
    xdo_keysequence_list_do(xdo, window, keymods, nkeymods, True, NULL);
    free(keymods);
  }

  return ret;
}

int cmd_windowmove(int argc, char **args) {
  int ret = 0;
  int x, y;
  Window wid;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid x y\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 3) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
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
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };

  int size_flags = 0;
  char *cmd = *args;
  int option_index;
  static const char* usage = "Usage: %s wid width height\n";

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  if (use_hints)
    size_flags |= SIZE_USEHINTS;

  args += optind;
  argc -= optind;

  if (argc != 3) {
    fprintf(stderr, usage, cmd);
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

int cmd_set_window(int argc, char** args) {
  char *cmd = *args;
  int c;
  char *role = NULL, *icon = NULL, *name = NULL, *class = NULL, *classname = NULL;

  struct option longopts[] = {
    { "name", required_argument, NULL, 'n' },
    { "icon-name", required_argument, NULL, 'i' },
    { "role", required_argument, NULL, 'r' },
    { "class", required_argument, NULL, 'C' },
    { "classname", required_argument, NULL, 'N' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  int option_index;
  static const char *usage = "Usage: %s [--name name] [--icon-name name] "
            "[--role role] [--classname classname] [--class class] wid\n";

  while ((c = getopt_long_only(argc, args, "hn:i:r:C:", longopts, &option_index)) != -1) {
    switch(c) {
      case 'n': 
        name = strdup(optarg); 
        break;
      case 'i':
        icon = strdup(optarg);
        break;
      case 'r':
        role = strdup(optarg);
        break;
      case 'C':
        class = strdup(optarg);
        break;
      case 'N':
        classname = strdup(optarg);
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }    
  }

  /* adjust argc, argv */
  args += optind;
  argc -= optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  Window wid  = (Window)strtol(args[0], NULL, 0);

  if (name)
    xdo_window_setprop(xdo, wid, "WM_NAME", name);
  if (icon)
    xdo_window_setprop(xdo, wid, "WM_ICON_NAME", icon);
  if (role)
    xdo_window_setprop(xdo, wid, "WM_WINDOW_ROLE", role);
  if (classname || class)
    xdo_window_setclass(xdo, wid, classname, class);

  return 0;
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
  int max_depth = -1;
  struct option longopts[] = {
    { "onlyvisible", 0, &only_visible, 1 },
    { "title", 0, &search_title, 1 },
    { "name", 0, &search_name, 1 },
    { "class", 0, &search_class, 1 },
    { "maxdepth", required_argument, NULL, 1 },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
      "Usage: xdotool %s "
      "[--onlyvisible --title --class --name --maxdepth N] regexp_pattern\n"
      " --onlyvisible   matches only windows currently visible\n"
      " --title         check regexp_pattern agains the window title\n"
      " --class         check regexp_pattern agains the window class\n"
      " --name          check regexp_pattern agains the window name\n"
      " --maxdepth <N>  set child window depth level to N. Default is infinite.\n"
      "                 -1 also means infinite.\n"
      " -h, --help      show this help output\n"
      "\n"
      "* If none of --title, --class, and --name are specified,\n"
      "the defaults are to match any of them.\n";

  int search_flags = 0;

  char *cmd = *args;
  int option_index;

  while (c = getopt_long_only(argc, args, "h", longopts, &option_index) != -1) {
    switch (c) {
      case 0:
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
      case 1:
        if (optarg) 
          max_depth = atoi(optarg);
        break;
      default:
        printf(usage, cmd);
        return 1;
    }
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
    printf(usage, cmd);
    return 1;
  }

  xdo_window_list_by_regex(xdo, *args, search_flags, max_depth, &list, &nwindows);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s -f\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc > 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (argc == 1) {
    if (!strcmp(args[0], "-f")) { /* -f was given */
      ret = xdo_window_get_focus(xdo, &wid);
    } else {
      fprintf(stderr, usage, cmd);
    }
  } else {
    /* No '-f' flag given, assume sane mode */
    ret = xdo_window_sane_get_focus(xdo, &wid);
  }

  if (ret) {
    fprintf(stderr, "xdo_window_focus reported an error\n");
  } else { 
    window_print(wid);
  }

  return ret;
}

int cmd_getactivewindow(int argc, char **args) {
  Window wid = 0;
  int ret;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 0) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ret = xdo_window_get_active(xdo, &wid);

  if (ret) {
    fprintf(stderr, "xdo_get_active_window reported an error\n");
  } else {
    window_print(wid);
  }

  return ret;
}

int cmd_windowmap(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
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
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_unmap(xdo, wid);
  if (ret)
    fprintf(stderr, "xdo_window_unmap reported an error\n");
  
  return ret;
}

int cmd_set_num_desktops(int argc, char **args) {
  char *cmd = *args;
  long ndesktops;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s num_desktops\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ndesktops = strtol(args[0], NULL, 0);

  return xdo_set_number_of_desktops(xdo, ndesktops);
}

int cmd_get_num_desktops(int argc, char **args) {
  int ret = 0;
  char *cmd = *args;
  long ndesktops = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 0) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ret = xdo_get_number_of_desktops(xdo, &ndesktops);

  printf("%ld\n", ndesktops);
  return ret;
}

int cmd_set_desktop(int argc, char **args) {
  char *cmd = *args;
  long desktop;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s desktop\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  desktop = strtol(args[0], NULL, 0);

  return xdo_set_current_desktop(xdo, desktop);
}

int cmd_get_desktop(int argc, char **args) {
  int ret = 0;
  char *cmd = *args;
  long desktop = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 0) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ret = xdo_get_current_desktop(xdo, &desktop);
  printf("%ld\n", desktop);
  return ret;
}

int cmd_set_desktop_for_window(int argc, char **args) {
  char *cmd = *args;
  long desktop = 0;
  Window window = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window> <desktop>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 2) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  window = (Window)strtol(args[0], NULL, 0);
  desktop = strtol(args[1], NULL, 0);

  return xdo_set_desktop_for_window(xdo, window, desktop);
}

int cmd_get_desktop_for_window(int argc, char **args) {
  int ret = 0;
  char *cmd = *args;
  long desktop = 0;
  Window window = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window>\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  window = (Window)strtol(args[0], NULL, 0);

  ret = xdo_get_desktop_for_window(xdo, window, &desktop);
  printf("%ld\n", desktop);
  return ret;
}

// vim:expandtab shiftwidth=2 softtabstop=2
