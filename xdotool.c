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
#include <ctype.h>
#include <stdarg.h>

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
void xdotool_debug(context_t *context, const char *format, ...);
void xdotool_output(context_t *context, const char *format, ...);

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
  { "getwindowgeometry", cmd_getwindowgeometry, },
  { "getdisplaygeometry", cmd_get_display_geometry, },
  { "search", cmd_search, },
  { "selectwindow", cmd_window_select, },

  /* Help me! */
  { "help", cmd_help, },
  { "version", cmd_version, },

  /* Action functions */
  { "behave", cmd_behave, },
  { "behave_screen_edge", cmd_behave_screen_edge, },
  { "click", cmd_click, },
  { "getmouselocation", cmd_getmouselocation, },
  { "key", cmd_key, },
  { "keydown", cmd_key, },
  { "keyup", cmd_key, },
  { "mousedown", cmd_mousedown, },
  { "mousemove", cmd_mousemove, },
  { "mousemove_relative", cmd_mousemove_relative, },
  { "mouseup", cmd_mouseup, },
  { "set_window", cmd_set_window, },
  { "type", cmd_type, },
  { "windowactivate", cmd_windowactivate, },
  { "windowfocus", cmd_windowfocus, },
  { "windowkill", cmd_windowkill, },
  { "windowclose", cmd_windowclose, },
  { "windowmap", cmd_windowmap, },
  { "windowminimize", cmd_windowminimize, },
  { "windowmove", cmd_windowmove, },
  { "windowraise", cmd_windowraise, },
  { "windowlower", cmd_windowlower, },
  { "windowreparent", cmd_windowreparent, },
  { "windowsize", cmd_windowsize, },
  { "windowunmap", cmd_windowunmap, },

  { "set_num_desktops", cmd_set_num_desktops, },
  { "get_num_desktops", cmd_get_num_desktops, },
  { "set_desktop", cmd_set_desktop, },
  { "get_desktop", cmd_get_desktop, },
  { "set_desktop_for_window", cmd_set_desktop_for_window, },
  { "get_desktop_for_window", cmd_get_desktop_for_window, },
  { "get_desktop_viewport", cmd_get_desktop_viewport, },
  { "set_desktop_viewport", cmd_set_desktop_viewport, },

  { "exec", cmd_exec, },
  { "sleep", cmd_sleep, },

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

  /* If argv[1] is a file or "-", read commands from file or stdin,
   * else use commands from argv.
   */

  struct stat data;
  int stat_ret;

  if (argc >= 2) {
    /* See if the first argument is an existing file */
    stat_ret = stat(argv[1], &data);
    int i = 0;
    int argv1_is_command= 0;

    for (i = 0; dispatch[i].name != NULL; i++) {
      if (!strcasecmp(dispatch[i].name, argv[1])) {
        argv1_is_command = 1;
        break;
      }
    }

    if (!argv1_is_command && (strcmp(argv[1], "-") == 0 || stat_ret == 0)) {
      return script_main(argc, argv);
    }
  }
  return args_main(argc, argv);
}

int script_main(int argc, char **argv) {
  /* Tokenize the input file while expanding positional parameters and
   * environment variables. Pass the resulting argument list to
   * args_main().
   */

  FILE *input = NULL;
  const char *path = argv[1];
  char buffer[4096];

  char **script_argv = (char **) calloc(1, sizeof(char *));
  int script_argc = 1;

  /* copy argv[0] to script_argv[0] */
  script_argv[0] = (char *) calloc(strlen(argv[0])+1, sizeof(char));
  strcpy(script_argv[0], argv[0]);

  /* determine whether reading from a file or from stdin */
  if (!strcmp(path, "-")) {
    input = fdopen(0, "r");
  } else {
    input = fopen(path, "r");
    if (input == NULL) {
      fprintf(stderr, "Failure opening '%s': %s\n", path, strerror(errno));
      return EXIT_FAILURE;
    }
  }

  /* read input... */
  int pos;
  char *token;

  while (fgets(buffer, 4096, input) != NULL) {
    char *line = buffer;
    token = NULL;

    /* Ignore leading whitespace */
    line += strspn(line, " \t");

    /* blanklines or line comment are ignored, too */
    if (line[0] == '\n' || line[0] == '#') {
      continue;
    }

    /* replace newline with null */
    if (line[strlen(line)-1] == '\n')
      line[strlen(line)-1] = '\0';

    /* tokenize line into script_argv... */
    while (strlen(line)) {
      token = NULL;

      /* modify line to contain the current token. Tokens are
       * separated by whitespace, or quoted with single/double quotes.
       */
      if (line[0] == '"') {
        line++;
        line[strcspn(line, "\"")] = '\0';
      }
      else if (line[0] == '\'') {
        line++;
        line[strcspn(line, "\'")] = '\0';
      }
      else {
        line[strcspn(line, " \t")] = '\0';
      }

      /* if a token begins with "$", append the corresponding
       * positional parameter or environment variable to
       * script_argv...
      */
      if (line[0] == '$') {
        /* ignore dollar sign */
        line++;

        if (isdigit(line[0])) {
          /* get the position of this parameter in argv */
          pos = atoi(line) + 1; /* $1 is actually index 2 in the argv array */

          /* bail if no argument was given for this parameter */
          if (pos >= argc) {
            fprintf (stderr, "%s: error: `%s' needs at least %d %s; only %d given\n",
                     argv[0], argv[1], pos - 1, pos == 2 ? "argument" : "arguments",
                     argc - 2);
            return EXIT_FAILURE;
          }
          /* use command line argument */
          token = argv[pos];
        }
        else {
          /* use environment variable */
          token = getenv(line);
          if (token == NULL) {
            /* since it's not clear what we should do if this env var is not
             * present, let's abort */
            fprintf(stderr, "%s: error: environment variable $%s is not set.\n",
                    argv[0], line);
            return EXIT_FAILURE;
          }
        }
      }
      else {
        /* use the verbatim token */
        token = line;
      }

      /* append token */
      if (token != NULL) {

        script_argv = realloc(script_argv, (script_argc+1) * sizeof(char *));
        if (script_argv == NULL) {
          fprintf(stderr, "%s: error: failed to allocate memory while parsing `%s'.\n",
                  argv[0], argv[1]);
          exit(EXIT_FAILURE);
        }
        script_argv[script_argc] = (char *) calloc(strlen(token)+1, sizeof(char));

        //printf("arg %d: %s\n", script_argc, token);
        strncpy(script_argv[script_argc], token, strlen(token)+1);
        script_argc++;
      }

      /* advance line to the next token */
      line += strlen(line)+1;
      line += strspn(line, " \t");
    }
  }
  fclose(input);

  /* Add NULL at the end */
  script_argv = realloc(script_argv, (script_argc+1) * sizeof(char *));
  /* TODO(sissel): STOPPED HERE */

  /* run the parsed script */
  int result = args_main(script_argc, script_argv);

  for(int i=0; i<script_argc+1; ++i) {
      free(script_argv[i]);
  }
  free(script_argv);
  return result;
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

  //for(i = 0; i<argc; i++) {
    //fprintf(stderr, "argv[%d] = \"%s\"\n", i, argv[i]);
  //}

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
  context.have_last_mouse = False;
  context.debug = (getenv("DEBUG") != NULL);

  if (context.xdo == NULL) {
    fprintf(stderr, "Failed creating new xdo instance\n");
    return 1;
  }
  context.xdo->debug = context.debug;

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
        if (context->debug) {
          fprintf(stderr, "command: %s\n", cmd);
        }
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
  xdotool_output(context, "xdotool version %s", xdo_version());
  if (context != NULL) {
    consume_args(context, 1);
  }

  return 0;
}

void xdotool_debug(context_t *context, const char *format, ...) {
  va_list args;

  va_start(args, format);
  if (context->debug) {
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
  }
} /* xdotool_debug */

void xdotool_output(context_t *context, const char *format, ...) {
  context = context; /* Do something with context to avoid warnings */
  va_list args;

  va_start(args, format);
  vfprintf(stdout, format, args);
  fprintf(stdout, "\n");
  fflush(stdout);
} /* xdotool_output */
