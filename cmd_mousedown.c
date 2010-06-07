#include "xdo_cmd.h"
#include <string.h>

int cmd_mousedown(context_t *context) {
  int ret = 0;
  int button;
  char *cmd = *context->argv;
  xdo_active_mods_t *active_mods = NULL;
  int clear_modifiers = 0;
  char *window_arg = NULL;

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

  while ((c = getopt_long_only(context->argc, context->argv, "chw:",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
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
    fprintf(stderr, "What button do you want me to send?\n");
    return EXIT_FAILURE;
  }

  button = atoi(context->argv[0]);

  window_each(context, window_arg, {
    if (clear_modifiers) {
      active_mods = xdo_get_active_modifiers(context->xdo);
      xdo_clear_active_modifiers(context->xdo, window, active_mods);
    }

    ret = xdo_mousedown(context->xdo, window, button);

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods);
      xdo_free_active_modifiers(active_mods);
    }

    if (ret) {
      fprintf(stderr, "xdo_mousedown reported an error on window %ld\n", window);
      return ret;
    }
  }); /* window_each(...) */

  consume_args(context, 1);

  return ret;
}
