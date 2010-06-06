#include "xdo_cmd.h"

int cmd_set_desktop_for_window(context_t *context) {
  char *cmd = *context->argv;
  long desktop = 0;
  Window window = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window> <desktop>\n";
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "h",
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

  if (context->argc < 2) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  window = (Window)strtol(context->argv[0], NULL, 0);
  desktop = strtol(context->argv[1], NULL, 0);
  consume_args(context, 2);
  return xdo_set_desktop_for_window(context->xdo, window, desktop);
}
