/* xdotool
 *
 * command line interface to the xdo library
 *
 * getwindowfocus contributed by Lee Pumphret
 * keyup/down contributed by Lee Pumphret
 *
 * vim:expandtab shiftwidth=2 softtabstop=2
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
#include "xdotool.h"

static int script_main(int argc, char **argv);
static int args_main(int argc, char **argv);
int context_execute(context_t *context);
void consume_args(context_t *context, int argc);
void window_save(context_t *context, Window window);
void window_list(context_t *context, const char *window_arg,
                 Window **windowlist_ret, int *nwindows_ret,
                 const int add_to_list);
int window_get_arg(context_t *context, int min_arg, int window_arg_pos,
                   const char **window_arg);
int window_is_valid(context_t *context, const char *window_arg);
int is_command(char* cmd);

void consume_args(context_t *context, int argc) {
  if (argc > context->argc) {
    fprintf(stderr,
            "Can't consume %d args; are only %d available. This is a bug.\n",
            argc, context->argc);
    context->argv += context->argc;
    context->argc = 0;
    return;
  }

  context->argv += argc;
  context->argc -= argc;
} /* void consume_args(context_t *, int) */

void window_save(context_t *context, Window window) {
  if (context->windows != NULL) {
    free(context->windows);
  }

  context->windows = calloc(1, sizeof(Window));
  context->nwindows = 1;
  context->windows[0] = window;
} /* void window_save(context_t *, Window) */

int window_is_valid(context_t *context, const char *window_arg) {
  if (window_arg == NULL) {
    return True;
  }

  if (window_arg[0] != '%') {
    return True;
  }

  /* Selected a window with %N or %@, but are there windows on the stack? */
  if (context->nwindows == 0) {
    fprintf(stderr, "There are no windows in the stack\n");
    return False;
  }

  if (window_arg[1] == '\0') {
    fprintf(stderr, "Invalid window stack selection '%s'\n", window_arg);
    return False;
  }

  if (window_arg[1] == '@') {
    return True;
  }

  int window_index = atoi(window_arg + 1);
  if (abs(window_index - 1) >= context->nwindows || (window_index == 0)) {
    fprintf(stderr, "Invalid window stack selection '%s' (out of range)\n", window_arg);
    return False;
  }

  return True;
} /* int window_is_valid(context_t *, const char *) */

int window_get_arg(context_t *context, int min_arg, int window_arg_pos,
                   const char **window_arg) {
  if (context->argc < min_arg) {
    fprintf(stderr, "Too few arguments (got %d, minimum is %d)\n",
            context->argc, min_arg);
    return False;
  } else if (context->argc == min_arg) {
    //fprintf(stderr, "Using default arg\n");
    /* nothing, keep default */
  } else if (context->argc > min_arg) {
    if (is_command(context->argv[min_arg])) {
      //fprintf(stderr, "arg is command, using default\n");
      /* keep default */
    } else {
      /* got enough args, let's use the window you asked for */
      //fprintf(stderr, "got enough args\n");
      *window_arg = context->argv[window_arg_pos];
      consume_args(context, 1);
    }
  }

  if (!window_is_valid(context, *window_arg)) {
    fprintf(stderr, "Invalid window '%s'\n", *window_arg);
    return False;
  }

  return True;
} /* int window_get_arg(context_t *, int, int, char **, int *) */

void window_list(context_t *context, const char *window_arg,
                 Window **windowlist_ret, int *nwindows_ret,
                 const int add_to_list) {
  /* If window_arg is NULL and we have windows in the list, use the list.
   * If window_arg is "%@" and we have windows in the list, use the list.
   * If window_arg is "%N" and we have windows in the list, use Nth window.
   *   'N' above must be a positive number.
   * Otherwise, assume it's a window id.
   *
   * TODO(sissel): Not implemented yet:
   * If window_arg is "%r" it means the root window of the current screen.
   * If window_arg is "%q" it means we will wait for you to select a window
   *   by clicking on it. (May not be necessary since we have 'selectwindow')
   * If window_arg is "%c" it means the currently-active window.
   */

  *nwindows_ret = 0;
  *windowlist_ret = NULL;

  if (window_arg != NULL && window_arg[0] == '%') {
    if (context->nwindows == 0) {
      fprintf(stderr, "There are no windows on the stack, Can't continue.\n");
      return;
    }

    if (strlen(window_arg) < 2) {
      fprintf(stderr, "Invalid window selection '%s'\n", window_arg);
      return;
    }

    /* options.
     * %N selects the Nth window. %1, %2, %-1 (last), %-2, etc.
     * %@ selects all
     */
    if (window_arg[1] == '@') {
      *windowlist_ret = context->windows;
      *nwindows_ret = context->nwindows;
    } else if (window_arg[1] == 'q') {
      /* TODO(sissel): Wait for you to click on the window. */
    } else if (window_arg[1] == 'r') {
      /* TODO(sissel): Get the root window of the current screen */
    } else if (window_arg[1] == 'c') {
      /* TODO(sissel): Get the current window */
    } else {
      /* Otherwise assume %N */
      int window_index = atoi(window_arg + 1);
      if (window_index < 0) {
        /* negative offset */
        window_index = context->nwindows + window_index;
      }

      if (window_index > context->nwindows || window_index <= 0) {
        fprintf(stderr, "%d is out of range (only %d windows in list)\n",
                window_index, context->nwindows);
        return;
      }

      /* Subtract 1 since %1 is the first window in the list */
      context->window_placeholder[0] = context->windows[window_index - 1];
      *windowlist_ret = context->window_placeholder;
      *nwindows_ret = 1;
    }
  } else {
    /* Otherwise, window_arg is either invalid or null. Default to CURRENTWINDOW
     */

    /* We can't return a pointer to a piece of the stack in this function,
     * so we'll store the window in the context_t and return a pointer
     * to that.
     */
    Window window = CURRENTWINDOW;
    if (window_arg != NULL) {
      window = (Window)strtol(window_arg, NULL, 0);
    }

    context->window_placeholder[0] = window;
    *nwindows_ret = 1;
    *windowlist_ret = context->window_placeholder;
  }

  if (add_to_list) {
    /* save the window to the windowlist */
  }
}


struct dispatch {
  const char *name;
  int (*func)(context_t *context);
} dispatch[] = {
  /* Query functions */
  { "getactivewindow", cmd_getactivewindow, },
  { "getwindowfocus", cmd_getwindowfocus, },
  { "getwindowname", cmd_getwindowname, },
  { "getwindowpid", cmd_getwindowpid, },
  { "search", cmd_search, },
  { "selectwindow", cmd_window_select, },

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
  { "windowreparent", cmd_windowreparent, },
  { "windowkill", cmd_windowkill, },
  { "set_window", cmd_set_window, },
  { "behave", cmd_behave, },
  { "behave_screen_edge", cmd_behave_screen_edge, },

  { "set_num_desktops", cmd_set_num_desktops, },
  { "get_num_desktops", cmd_get_num_desktops, },
  { "set_desktop", cmd_set_desktop, },
  { "get_desktop", cmd_get_desktop, },
  { "set_desktop_for_window", cmd_set_desktop_for_window, },
  { "get_desktop_for_window", cmd_get_desktop_for_window, },
  { "get_desktop_viewport", cmd_get_desktop_viewport, },
  { "set_desktop_viewport", cmd_set_desktop_viewport, },

  { NULL, NULL, },
};

int is_command(char* cmd) {
  int i;
  for (i = 0; dispatch[i].name != NULL; i++) {
      if (!strcasecmp(dispatch[i].name, cmd)) {
	return 1;
      }
    }
  return 0;
}

int main(int argc, char **argv) {
  return xdotool_main(argc, argv);
}

int xdotool_main(int argc, char **argv) {
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

    if (asprintf(&cmd, "%s %s", argv[0], line) == -1) {
      fprintf(stderr, "asprintf failed\n");
      exit(1);
    }
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
  int ret = 0;
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
    cmd_help(NULL);
    exit(1);
  }

  while ((opt = getopt_long_only(argc, argv, "++hv", long_options, &option_index)) != -1) {
    switch (opt) {
      case 'h':
        cmd_help(NULL);
        exit(EXIT_SUCCESS);
      case 'v':
        cmd_version(NULL);
        exit(EXIT_SUCCESS);
      default:
        fprintf(stderr, usage, argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  context_t context;
  context.xdo = xdo_new(NULL);
  context.prog = *argv;
  argv++; argc--;
  context.argc = argc;
  context.argv = argv;
  context.windows = NULL;
  context.nwindows = 0;

  if (context.xdo == NULL) {
    fprintf(stderr, "Failed creating new xdo instance\n");
    return 1;
  }

  ret = context_execute(&context);

  xdo_free(context.xdo);
  if (context.windows != NULL) {
    free(context.windows);
  }

  return ret;
} /* int args_main(int, char **) */

int context_execute(context_t *context) {
  int cmd_found = 0;
  int i = 0;
  char *cmd = NULL;
  int ret = XDO_SUCCESS;

  /* Loop until all argv is consumed. */
  while (context->argc > 0 && ret == XDO_SUCCESS) {
    cmd = context->argv[0];
    cmd_found = 0;
    for (i = 0; dispatch[i].name != NULL && !cmd_found; i++) {
      if (!strcasecmp(dispatch[i].name, cmd)) {
        cmd_found = 1;
        optind = 0;
        ret = dispatch[i].func(context);
      }
    }

    if (!cmd_found) {
      fprintf(stderr, "%s: Unknown command: %s\n", context->prog, cmd);
      fprintf(stderr, "Run '%s help' if you want a command list\n", context->prog);
      ret = 1;
    }
  } /* while ... */
  return ret;
} /* int args_main(int, char **) */

int cmd_help(context_t *context) {
  int i;
  printf("Available commands:\n");
  for (i = 0; dispatch[i].name != NULL; i++)
    printf("  %s\n", dispatch[i].name);

  /* "help" can be invoked on errors, like when xdotool is given no arguments,
   * so let's make sure we only consume if we have a context */
  if (context != NULL) {
    consume_args(context, 1);
  }

  return 0;
}

int cmd_version(context_t *context) {
  printf("xdotool version %s\n", xdo_version());
  if (context != NULL) {
    consume_args(context, 1);
  }

  return 0;
}
