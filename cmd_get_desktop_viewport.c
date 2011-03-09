#include "xdo_cmd.h"

int cmd_get_desktop_viewport(context_t *context) {
  int ret = 0;
  char *cmd = context->argv[0];
  int shell_output = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "shell", no_argument, NULL, 's' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 's':
        shell_output = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  int x, y;
  consume_args(context, optind);
  ret = xdo_get_desktop_viewport(context->xdo, &x, &y);

  if (shell_output) {
    xdotool_output(context, "X=%d", x);
    xdotool_output(context, "Y=%d", y);
  } else {
    xdotool_output(context, "%d %d", x, y);
  }

  return ret;
}
