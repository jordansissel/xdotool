#include "xdo_cmd.h"

int cmd_windowraise(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s window\n";
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

  if (context->argc < 1) {
    fprintf(stderr, usage, cmd);
    return 1;
   }

  window_each(context, context->argv[0], {
    ret = xdo_window_raise(context->xdo, window);
    if (ret) {
      fprintf(stderr, "xdo_window_raise reported an error on window %ld\n",
              window);
    }
  }); /* window_each(...) */

  consume_args(context, 1);
  return ret;
}

