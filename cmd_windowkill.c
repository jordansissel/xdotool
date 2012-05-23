#include "xdo_cmd.h"

int cmd_windowkill(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  const char *window_arg = "%1";

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [window=%1]\n"
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

  if (!window_get_arg(context, 0, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  window_each(context, window_arg, {
    ret = xdo_kill_window(context->xdo, window);
    if (ret) {
      fprintf(stderr, "xdo_kill_window reported an error on window %ld\n",
              window);
    }
  }); /* window_each(...) */

  return ret;
} /* int cmd_windowkill(context_t *) */

