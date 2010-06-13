#include "xdo_cmd.h"
#include <string.h>

int cmd_mouseup(context_t *context) {
  int ret = 0;
  int button;
  char *cmd = *context->argv;
  char *window_arg = NULL;
  xdo_active_mods_t *active_mods = NULL;
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

  while ((c = getopt_long_only(context->argc, context->argv, "+cw:h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'w':
        window_arg = strdup(optarg);
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc < 1) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(context->argv[0]);

  window_each(context, window_arg, {
    if (clear_modifiers) {
      active_mods = xdo_get_active_modifiers(context->xdo);
      xdo_clear_active_modifiers(context->xdo, window, active_mods);
    }

    ret = xdo_mouseup(context->xdo, window, button);

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods);
      xdo_free_active_modifiers(active_mods);
    }

    if (ret) {
      fprintf(stderr, "xdo_mouseup reported an error on window %ld\n", window);
      return ret;
    }
  }); /* window_each(...) */

  if (window_arg != NULL) {
    free(window_arg);
  }
  consume_args(context, 1);
  return ret;
}

