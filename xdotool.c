/* xdotool
 *
 * command line interface to the xdo library
 *
 * getwindowfocus contributed by Lee Pumphret
 * keyup/down contributed by Lee Pumphret
 *
 * XXX: Need to use 'Window' instead of 'int' where appropriate.
 */

#define _GNU_SOURCE 1
#ifndef __USE_BSD
#define __USE_BSD /* for strdup on linux/glibc */
#endif /* __USE_BSD */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "xdo.h"

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

xdo_t *xdo;
void window_print(Window wid);
static int script_main(int argc, char **argv);
static int args_main(int argc, char **argv);

struct dispatch {
  const char *name;
  int (*func)(int argc, char **args);
} dispatch[] = {
  /* Query functions */
  { "getactivewindow", cmd_getactivewindow, },
  { "getwindowfocus", cmd_getwindowfocus, },
  { "getwindowpid", cmd_getwindowpid, },
  { "search", cmd_search, },

  /* Help me! */
  { "help", cmd_help, },
  { "version", cmd_version, },

  /* Action functions */
  { "click", cmd_click, },
  { "getmouselocation", cmd_getmouselocation, },
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
  /* read stdin if stdin is not a tty or first argument is "-" */

  int want_script;
  struct stat data;
  int stat_ret;

  /* If we are being run from a script with shebang line #!/path/to/xdotool
   * then the input file will be argv[argc - 1].
   */
  stat_ret = stat(argv[argc - 1], &data);

  want_script = ((argc == 2 && !strcmp(argv[1], "-"))
                 || (argc == 2 && stat_ret == 0));
  if (want_script) {
    return script_main(argc, argv);
  } else {
    return args_main(argc, argv);
  }
}

int script_main(int argc, char **argv) {
  FILE *input = NULL;
  const char *path = argv[argc - 1];
  char *cmd;
  char buffer[4096];

  if (!strcmp(path, "-") || !isatty(0)) {
    input = fdopen(0, "r");
  } else {
    input = fopen(path, "r");
    if (input == NULL) {
      fprintf(stderr, "Failure opening '%s': %s\n", path, strerror(errno));
      return EXIT_FAILURE;
    }
  }

  int ret;
  while (fgets(buffer, 4096, input) != NULL) {
    char *line = buffer;
    // Ignore leading whitespace
    line += strspn(line, " \t");

    // blanklines or line comment are ignored, too
    if (line[0] == '\n' || line[0] == '#') {
      continue;
    }
    line[strlen(line) - 1] = '\0'; /* replace newline with null */

    asprintf(&cmd, "%s %s", argv[0], line);
    //printf("Running: %s\n", cmd);
    ret = system(cmd);
    free(cmd);

    if (ret != EXIT_SUCCESS) {
      return ret;
    }
  }

  return EXIT_SUCCESS;
}

int args_main(int argc, char **argv) {
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
    { "version", no_argument, NULL, 'v' },
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
      case 'v':
        cmd_version(0, NULL);
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
    fprintf(stderr, "%s: Unknown command: %s\n", strrchr(prog, '/') + 1, cmd);
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

int cmd_version(int unused_argc, char **unused_args) {
  printf("xdotool version %s\n", xdo_version());
  return 0;
}

int cmd_mousemove(int argc, char **args) {
  int ret = 0;
  int x, y;
  char *cmd = *args;
  xdo_active_mods_t *active_mods;
  int clear_modifiers = 0;

  int c;
  int screen = 0;
  Window window = 0;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "screen", required_argument, NULL, 's' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
             "Usage: %s [options] <xcoord> <ycoord>\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
            "--screen SCREEN        - which screen to move on, default is current screen\n"
            "--window <windowid>    - specify a window to send keys to\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "chs:w:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 's':
        screen = atoi(optarg);
        break;
      case 'w':
        window = strtoul(optarg, NULL, 0);
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

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  if (window > 0) {
    ret = xdo_mousemove_relative_to_window(xdo, window, x, y);
  } else {
    ret = xdo_mousemove(xdo, x, y, screen);
  }

  if (ret)
    fprintf(stderr, "xdo_mousemove reported an error\n");

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

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
  Window window = 0;
  xdo_active_mods_t *active_mods;
  int clear_modifiers = 0;

  int c;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
            "Usage: %s [--clearmodifiers] [--window WINDOW] <button>\n"
            "--window <windowid>    - specify a window to send keys to\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "chw:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 'w':
        window = strtoul(optarg, NULL, 0);
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

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  ret = xdo_mousedown(xdo, window, button);

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  if (ret)
    fprintf(stderr, "xdo_mousedown reported an error\n");
  return ret;
}

int cmd_mouseup(int argc, char **args) {
  int ret = 0;
  int button;
  char *cmd = *args;
  Window window = 0;
  xdo_active_mods_t *active_mods;
  int clear_modifiers = 0;

  int c;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
            "Usage: %s [--clearmodifiers] [--window WINDOW] <button>\n"
            "--window <windowid>    - specify a window to send keys to\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "cw:h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'w':
        window = strtoul(optarg, NULL, 0);
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

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  ret = xdo_mouseup(xdo, window, button);

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

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
    { "shell", no_argument, NULL, 's' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
  int option_index;
  int output_shell = 0;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 's':
        output_shell = 1;
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

  if (output_shell) {
    printf("X=%d\n", x);
    printf("Y=%d\n", y);
    printf("SCREEN=%d\n", screen_num);
  } else {
    printf("x:%d y:%d screen:%d\n", x, y, screen_num);
  }
  return ret;
}

int cmd_click(int argc, char **args) {
  int button;
  char *cmd = *args;
  int ret;
  int clear_modifiers = 0;
  Window window = 0;
  xdo_active_mods_t *active_mods;

  int c;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
            "Usage: %s [--clearmodifiers] [--window WINDOW] <button>\n"
            "--window <windowid>    - specify a window to send keys to\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "cw:h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'w':
        window = strtoul(optarg, NULL, 0);
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

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  ret = xdo_click(xdo, window, button);

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret;
}

int cmd_type(int argc, char **args) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *args;
  xdo_active_mods_t *active_mods;

  /* Options */
  int clear_modifiers = 0;
  Window window = 0;
  useconds_t delay = 12000; /* 12ms between keystrokes default */

  struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "delay", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
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
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  for (i = 0; i < argc; i++) {
    int tmp = xdo_type(xdo, window, args[i], delay);

    if (tmp) {
      fprintf(stderr, "xdo_type reported an error\n");
    }

    ret += tmp;
  }

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret > 0;
}

int cmd_key(int argc, char **args) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *args;
  xdo_active_mods_t *active_mods;

  /* Options */
  Window window = 0;
  int clear_modifiers = 0;

  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };

  static const char *usage = 
             "Usage: %s [--window windowid] [--clearmodifiers] <keyseq1> [keyseq2 ... keyseqN]\n"
             "Each keysequence can be any number of modifiers and keys, separated by plus (+)\n"
             "For example: alt+r\n"
             "Any letter or key symbol such as Shift_L, Return, Dollar, a, space are valid here.\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "hcw:", longopts, &option_index)) != -1) {
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

  int (*func)(const xdo_t *, Window, const char *) = NULL;

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
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  for (i = 0; i < argc; i++) {
    int tmp = func(xdo, window, args[i]);
    if (tmp != 0)
      fprintf(stderr, "xdo_keysequence reported an error for string '%s'\n", args[i]);
    ret += tmp;
  }

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
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
    { "usehints", 0, NULL, 'u' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };

  int size_flags = 0;
  char *cmd = *args;
  int option_index;
  static const char *usage =
            "Usage: %s [--usehints] windowid width height\n" \
            "--usehints  - Use window sizing hints (like characters in terminals)\n";

  while ((c = getopt_long_only(argc, args, "uh", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
      case 'u':
        use_hints = 1;
        break;
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
  xdo_search_t search;
  int nwindows;
  int i;
  int c;

  int search_title = -1;
  int search_name = -1;
  int search_class = -1;
  int search_classname = -1;
  typedef enum { 
    opt_unused, opt_title, opt_onlyvisible, opt_name, opt_class, opt_maxdepth,
    opt_pid, opt_help, opt_any, opt_all, opt_screen, opt_classname
  } optlist_t;
  struct option longopts[] = {
    { "all", no_argument, NULL, opt_all },
    { "any", no_argument, NULL, opt_any },
    { "class", no_argument, &search_class, opt_class },
    { "classname", no_argument, &search_class, opt_classname },
    { "help", no_argument, NULL, opt_help },
    { "maxdepth", required_argument, NULL, opt_maxdepth },
    { "name", no_argument, NULL, opt_name },
    { "onlyvisible", 0, NULL, opt_onlyvisible },
    { "pid", required_argument, NULL, opt_pid },
    { "screen", required_argument, NULL, opt_screen },
    { "title", no_argument, NULL, opt_title },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
      "Usage: xdotool %s "
      "[options] regexp_pattern\n"
      " --class         check regexp_pattern agains the window class\n"
      " --classname     check regexp_pattern agains the window classname\n"
      " --maxdepth N    set search depth to N. Default is infinite.\n"
      " --name          check regexp_pattern agains the window name\n"
      " --onlyvisible   matches only windows currently visible\n"
      " --pid PID       only show windows belonging to specific process\n"
      "                 Not supported by all X11 applications"
      " --screen N      only search a specific screen. Default is all screens\n"
      " --title         check regexp_pattern agains the window title\n"
      "                 -1 also means infinite.\n"
      " -h, --help      show this help output\n"
      "\n"
      "* If none of --title, --class, and --name are specified,\n"
      "the defaults are to match any of them.\n";

  memset(&search, 0, sizeof(xdo_search_t));
  search.max_depth = -1;
  search.require = SEARCH_ANY;

  char *cmd = *args;
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 0:
        break;
      case opt_help:
        printf(usage, cmd);
        return EXIT_SUCCESS;
      case opt_maxdepth:
        search.max_depth = strtol(optarg, NULL, 0);
        break;
      case opt_pid:
        search.pid = atoi(optarg);
        search.searchmask |= SEARCH_PID;
        break;
      case opt_any:
        search.require = SEARCH_ANY;
        break;
      case opt_all:
        search.require = SEARCH_ALL;
        break;
      case opt_screen:
        search.screen = strtoul(optarg, NULL, 0);
        search.searchmask |= SEARCH_SCREEN;
        break;
      case opt_onlyvisible:
        search.only_visible = True;
        search.searchmask |= SEARCH_ONLYVISIBLE;
        break;
      case opt_title:
        search_title = True;
        break;
      case opt_class:
        search_class = True;
        break;
      case opt_classname:
        search_classname = True;
        break;
      case opt_name:
        search_name = True;
        break;
      default:
        printf("Invalid usage\n");
        printf(usage, cmd);
        return EXIT_FAILURE;
    }
  }

  args += optind;
  argc -= optind;

  /* We require a pattern or a pid to search for */
  if (argc != 1 && search.pid == 0) {
    printf(usage, cmd);
    return EXIT_FAILURE;
  }

  if (search_title < 0 && search_name < 0 && search_class 
      && search_classname < 0 && argc > 0) {
    fprintf(stderr, "Defaulting to search window title, class, classname, "
            "and name (title)\n");
    search.searchmask |= (SEARCH_TITLE | SEARCH_NAME | SEARCH_CLASS | SEARCH_CLASSNAME);
    search_title = opt_title;
    search_name = opt_name;
    search_class = opt_class;
  }

  if (argc > 0) {
    if (search_title) {
      search.searchmask |= SEARCH_TITLE;
      search.title = args[0];
    }
    if (search_name) {
      search.searchmask |= SEARCH_NAME;
      search.winname = args[0];
    }
    if (search_class) {
      search.searchmask |= SEARCH_CLASS;
      search.winclass = args[0];
    }
    if (search_classname) {
      search.searchmask |= SEARCH_CLASSNAME;
      search.winclassname = args[0];
    }
  }

  xdo_window_search(xdo, &search, &list, &nwindows);
  for (i = 0; i < nwindows; i++)
    window_print(list[i]);

  /* Free list as it's malloc'd by xdo_window_search */
  free(list);

  /* error if number of windows found is zero (behave like grep) */
  return (nwindows ? EXIT_SUCCESS : EXIT_FAILURE);
}

/* Added 2007-07-28 - Lee Pumphret */
int cmd_getwindowfocus(int argc, char **args) {
  int ret = 0;
  int get_toplevel_focus = 1;
  Window wid = 0;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 'f' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s -f\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "fh", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 'f':
        get_toplevel_focus = 0;
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

  if (get_toplevel_focus) {
    ret = xdo_window_sane_get_focus(xdo, &wid);
  } else {
    ret = xdo_window_get_focus(xdo, &wid);
  }

  if (ret) {
    fprintf(stderr, "xdo_window_focus reported an error\n");
  } else { 
    window_print(wid);
  }

  return ret;
}

int cmd_getwindowpid(int argc, char **args) {
  Window wid = 0;
  int pid;
  char *cmd = *args;

  int c;

  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window id>\n";
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
  pid = xdo_window_get_pid(xdo, wid);
  if (pid == 0) {
    fprintf(stderr, "window %ld has no pid associated with it.\n", wid);
  }

  printf("%d\n", pid);
  return pid != 0;
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
