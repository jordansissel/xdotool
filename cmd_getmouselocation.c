#include "xdo_cmd.h"

int cmd_getmouselocation(context_t *context) {
  int x, y, screen_num;
  Window window;
  int ret;
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "shell", no_argument, NULL, 's' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [--shell]\n"
    "--shell     - output shell variables for use with eval\n";
  int option_index;
  int output_shell = 0;

  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 's':
        output_shell = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  ret = xdo_mouselocation2(context->xdo, &x, &y, &screen_num, &window);

  if (output_shell) {
    xdotool_output(context, "X=%d", x);
    xdotool_output(context, "Y=%d", y);
    xdotool_output(context, "SCREEN=%d", screen_num);
    xdotool_output(context, "WINDOW=%d", window);
  } else {
    xdotool_output(context, "x:%d y:%d screen:%d window:%ld", x, y, screen_num, window);
  }
  return ret;
}

