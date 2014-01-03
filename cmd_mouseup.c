#include "xdo_cmd.h"
#include <string.h>

int cmd_mouseup(context_t *context) {
  int ret = EXIT_FAILURE;
  int button;
  char *cmd = *context->argv;
  char *window_arg = NULL;
  charcodemap_t *active_mods = NULL;
  int active_mods_n;
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
        ret = EXIT_SUCCESS;
        goto finalize;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'w':
        free(window_arg);
        window_arg = strdup(optarg);
        break;
      default:
        fprintf(stderr, usage, cmd);
        goto finalize;
    }
  }

  consume_args(context, optind);

  if (context->argc < 1) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "What button do you want me to release?\n");
    goto finalize;
  }

  button = atoi(context->argv[0]);

  ret = EXIT_SUCCESS;

  window_each(context, window_arg, {
    if (clear_modifiers) {
      xdo_get_active_modifiers(context->xdo, &active_mods, &active_mods_n);
      xdo_clear_active_modifiers(context->xdo, window, active_mods, active_mods_n);
    }

    ret = xdo_mouse_up(context->xdo, window, button);

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods, active_mods_n);
      free(active_mods);
    }

    if (ret) {
      fprintf(stderr, "xdo_mouse_up reported an error on window %ld\n", window);
      goto finalize;
    }
  }); /* window_each(...) */

  consume_args(context, 1);
finalize:
  free(window_arg);
  return ret;
}

