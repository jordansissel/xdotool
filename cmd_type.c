#include "xdo_cmd.h"
#include <string.h>

int cmd_type(context_t *context) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *context->argv;
  char *window_arg = NULL;
  int arity = -1;
  char *terminator = NULL;

  char **data = NULL; /* stuff to type */
  int data_count = 0;
  xdo_active_mods_t *active_mods = NULL;

  /* Options */
  int clear_modifiers = 0;
  useconds_t delay = 12000; /* 12ms between keystrokes default */

  typedef enum {
    opt_unused, opt_clearmodifiers, opt_delay, opt_help, opt_window, opt_args,
    opt_terminator
  } optlist_t;

  struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, opt_clearmodifiers },
    { "delay", required_argument, NULL, opt_delay },
    { "help", no_argument, NULL, opt_help },
    { "window", required_argument, NULL, opt_window },
    { "args", required_argument, NULL, opt_args },
    { "terminator", required_argument, NULL, opt_terminator },
    { 0, 0, 0, 0 },
  };

  static const char *usage =
    "Usage: %s [--window windowid] [--delay milliseconds] "
    "<things to type>\n"
    "--window <windowid>    - specify a window to send keys to\n"
    "--delay <milliseconds> - delay between keystrokes\n"
    "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
    "--args N  - how many arguments to expect in the exec command. This is\n"
    "            useful for ending an exec and continuing with more xdotool\n"
    "            commands\n"
    "--terminator TERM - similar to --args, specifies a terminator that\n"
    "                    marks the end of 'exec' arguments. This is useful\n"
    "                    for continuing with more xdotool commands.\n"
            "-h, --help             - show this help output\n";
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+w:d:ch",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case opt_window:
        window_arg = strdup(optarg);
        break;
      case opt_delay:
        /* --delay is in milliseconds, convert to microseconds */
        delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case opt_clearmodifiers:
        clear_modifiers = 1;
        break;
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case opt_args:
        arity = atoi(optarg);
        break;
      case opt_terminator:
        terminator = strdup(optarg);
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc == 0) {
    fprintf(stderr, "You specified the wrong number of args.\n");
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (arity > 0 && terminator != NULL) {
    fprintf(stderr, "Don't use both --terminator and --args.\n");
    return EXIT_FAILURE;
  }

  if (context->argc < arity) {
    fprintf(stderr, "You said '--args %d' but only gave %d arguments.\n",
            arity, context->argc);
    return EXIT_FAILURE;
  }

  /* Apply any --arity or --terminator */
  data = calloc(context->argc, sizeof(char *));
  for (i=0; i < context->argc; i++) {
    if (arity > 0 && i == arity) {
      data[i] = NULL;
      break;
    }

    /* if we have a terminator and the current argument matches it... */
    if (terminator != NULL && strcmp(terminator, context->argv[i]) == 0) {
      data[i] = NULL;
      data_count++; /* Consume the terminator, too */
      break;
    }

    data[i] = strdup(context->argv[i]);
    data_count = i + 1;
    xdotool_debug(context, "Exec arg[%d]: %s", i, data[i]);
  }

  window_each(context, window_arg, {
    if (clear_modifiers) {
      active_mods = xdo_get_active_modifiers(context->xdo);
      xdo_clear_active_modifiers(context->xdo, window, active_mods);
    }

    for (i = 0; i < data_count; i++) {
      //printf("Typing: '%s'\n", context->argv[i]);
      int tmp = xdo_type(context->xdo, window, data[i], delay);

      if (tmp) {
        fprintf(stderr, "xdo_type reported an error\n");
      }

      ret += tmp;
    }

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods);
      xdo_free_active_modifiers(active_mods);
    }
  }); /* window_each(...) */

  if (window_arg != NULL) {
    free(window_arg);
  }

  consume_args(context, data_count);
  return ret > 0;
}

