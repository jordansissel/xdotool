#include "xdo_cmd.h"

int cmd_click(context_t *context) {
  int button;
  char *cmd = context->argv[0];
  int ret;
  int clear_modifiers = 0;
  Window window = 0;
  xdo_active_mods_t *active_mods = NULL;

  int c;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
            "Usage: %s [options] <button>\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
            "--window <windowid>    - specify a window to send click to\n"
            "\n"
            "Button is a button number. Generally, left = 1, middle = 2, \n"
            "right = 3, wheel up = 4, wheel down = 5\n";
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "cw:h",
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
        window = strtoul(optarg, NULL, 0);
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
    return EXIT_FAILURE;
  }

  button = atoi(context->argv[0]);

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(context->xdo);
    xdo_clear_active_modifiers(context->xdo, window, active_mods);
  }

  ret = xdo_click(context->xdo, window, button);

  if (clear_modifiers) {
    xdo_set_active_modifiers(context->xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  consume_args(context, 1);
  return ret;
}
