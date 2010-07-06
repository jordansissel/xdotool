#include "xdo_cmd.h"

int cmd_set_desktop_viewport(context_t *context) {
  int ret = 0;
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s x y\n";
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
  int x, y;

  if (context->argc < 2) {
    fprintf(stderr, "Not enough arguments given.\n");
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  x = atoi(context->argv[0]);
  y = atoi(context->argv[1]);
  consume_args(context, 2);

  ret = xdo_set_desktop_viewport(context->xdo, x, y);
  return ret;
}
