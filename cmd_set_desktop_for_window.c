#include "xdo_cmd.h"

int cmd_set_desktop_for_window(context_t *context) {
  char *cmd = *context->argv;
  long desktop = 0;
  int ret = EXIT_SUCCESS;
  const char *window_arg = "%1";

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [window=%1] <desktop>\n"
    HELP_SEE_WINDOW_STACK;
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (!window_get_arg(context, 1, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  desktop = strtol(context->argv[0], NULL, 0);
  consume_args(context, 1);

  window_each(context, window_arg, {
    ret = xdo_set_desktop_for_window(context->xdo, window, desktop);
    if (ret != XDO_SUCCESS) {
      fprintf(stderr, 
              "xdo_set_desktop_for_window on window %ld, desktop %ld failed\n", 
              window, desktop);
      return ret;
    }
  }); /* window_each(...) */

  return ret;
}
