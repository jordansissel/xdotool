#include "xdo_cmd.h"
#include <string.h> /* for strcmp */

/* This function handles all of these commands:
 * xdotool key
 * xdtoool keyup
 * xdotool keydown
 */

int cmd_key(context_t *context) {
  int ret = 0;
  int i, j;
  int c;
  char *cmd = *context->argv;
  charcodemap_t *active_mods = NULL;
  int active_mods_n;
  useconds_t key_delay = 12000;
  useconds_t repeat_delay = 0;
  int repeat = 1;
  const char *window_arg = NULL;
  int free_arg = 0;

  /* Options */
  int clear_modifiers = 0;

  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "delay", required_argument, NULL, 'd' },
    { "repeat-delay", required_argument, NULL, 'R' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { "repeat", required_argument, NULL, 'r' },
    { 0, 0, 0, 0 },
  };

  static const char *usage = 
     "Usage: %s [options] <keysequence> [keysequence ...]\n"
     "--clearmodifiers     - clear active keyboard modifiers during keystrokes\n"
     "--delay DELAY        - Use DELAY milliseconds between keystrokes\n"
     "--repeat TIMES       - How many times to repeat the key sequence\n"
     "--repeat-delay DELAY - DELAY milliseconds between repetitions\n"
     "--window WINDOW      - send keystrokes to a specific window\n"
     "Each keysequence can be any number of modifiers and keys, separated by plus (+)\n"
     "  For example: alt+r\n"
     "\n"
     "Any letter or key symbol such as Shift_L, Return, Dollar, a, space are valid,\n"
     "including those not currently available on your keyboard.\n"
     "\n"
     "If no window is given, and there are windows in the stack, %1 is used. Otherwise\n"
     "the currently-focused window is used\n";

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
        key_delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case 'r':
        repeat = atoi(optarg);
        if (repeat < 1) {
          fprintf(stderr, "Invalid '--repeat' value given: %s\n", optarg);
          return EXIT_FAILURE;
        }
        break;
      case 'R': // --repeat-delay
        /* Argument is in milliseconds, keysequence delay is in microseconds. */
        repeat_delay = strtoul(optarg, NULL, 0) * 1000;
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
    keyfunc = xdo_send_keysequence_window;
  } else if (!strcmp(cmd, "keyup")) {
    keyfunc = xdo_send_keysequence_window_up;
  } else if (!strcmp(cmd, "keydown")) {
    keyfunc = xdo_send_keysequence_window_down;
  } else {
    fprintf(stderr, "Unknown command '%s'\n", cmd);
    return 1;
  }

  int max_arg = context->argc;
  window_each(context, window_arg, {
    if (clear_modifiers) {
      xdo_get_active_modifiers(context->xdo, &active_mods, &active_mods_n);
      xdo_clear_active_modifiers(context->xdo, window, active_mods, active_mods_n);
    }

    for (j = 0; j < repeat; j++) {
      for (i = 0; i < context->argc; i++) {
        if (is_command(context->argv[i])) {
          max_arg = i;
          break;
        }
        int tmp = keyfunc(context->xdo, window, context->argv[i], key_delay);
        if (tmp != 0) {
          fprintf(stderr,
                  "xdo_send_keysequence_window reported an error for string '%s'\n",
                  context->argv[i]);
        }
        ret += tmp;
      } /* each keysequence */

      /* Sleep if --repeat-delay given and not on the last repetition */
      if (repeat_delay > 0 && j < (repeat-1))  {
        usleep(repeat_delay);
      }
    } /* repeat */

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods, active_mods_n);
      free(active_mods);
    }
  }); /* window_each(...) */

  if (free_arg) {
    free((char *)window_arg);
  }

  consume_args(context, max_arg);

  return ret;
}

