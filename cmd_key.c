#include "xdo_cmd.h"
#include <string.h> /* for strcmp */

/* This function handles all of these commands:
 * xdotool key
 * xdtoool keyup
 * xdotool keydown
 */

int cmd_key(context_t *context) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *context->argv;
  xdo_active_mods_t *active_mods = NULL;
  useconds_t delay = 12000;
  const char *window_arg = NULL;
  int free_arg = 0;

  /* Options */
  int clear_modifiers = 0;

  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "delay", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };

  static const char *usage = 
     "Usage: %s [options] <keysequence> [keysequence ...]\n"
     "--clearmodifiers     - clear active keyboard modifiers during keystrokes\n"
     "--delay DELAY        - Use DELAY milliseconds between keystrokes\n"
     "--window WINDOW      - send keystrokes to a specific window\n"
     "Each keysequence can be any number of modifiers and keys, separated by plus (+)\n"
     "  For example: alt+r\n"
     "\n"
     "Any letter or key symbol such as Shift_L, Return, Dollar, a, space are valid,\n"
     "including those not currently available on your keyboard.\n"
     "\n"
     "If no window is given, and there are windows in the stack, %1 is used. Otherwise\n"
     "the currently-focused window is used\n"
     HELP_CHAINING_ENDS;
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+d:hcw:",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'w':
        window_arg = strdup(optarg);
        free_arg = 1;
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 'd':
        /* Argument is in milliseconds, keysequence delay is in microseconds. */
        delay = strtoul(optarg, NULL, 0) * 1000;
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

  /* use %1 if there is a window stack */
  if (window_arg == NULL && context->nwindows > 0) {
    window_arg = "%1";
  }

  int (*keyfunc)(const xdo_t *, Window, const char *, useconds_t) = NULL;

  if (!strcmp(cmd, "key")) {
    keyfunc = xdo_keysequence;
  } else if (!strcmp(cmd, "keyup")) {
    keyfunc = xdo_keysequence_up;
  } else if (!strcmp(cmd, "keydown")) {
    keyfunc = xdo_keysequence_down;
  } else {
    fprintf(stderr, "Unknown command '%s'\n", cmd);
    return 1;
  }

  int max_arg = context->argc;
  window_each(context, window_arg, {
    if (clear_modifiers) {
      active_mods = xdo_get_active_modifiers(context->xdo);
      xdo_clear_active_modifiers(context->xdo, window, active_mods);
    }

    for (i = 0; i < context->argc; i++) {
      if (is_command(context->argv[i])) {
        max_arg = i + 1;
        break;
      }
      int tmp = keyfunc(context->xdo, window, context->argv[i], delay);
      if (tmp != 0) {
        fprintf(stderr,
                "xdo_keysequence reported an error for string '%s'\n",
                context->argv[i]);
      }
      ret += tmp;
    }

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods);
      xdo_free_active_modifiers(active_mods);
    }
  }); /* window_each(...) */

  if (free_arg) {
    free((char *)window_arg);
  }

  consume_args(context, max_arg);

  return ret;
}

