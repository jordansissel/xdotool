#include "xdo_cmd.h"

int cmd_getactivewindow(context_t *context) {
  Window window = 0;
  int ret;
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
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

  //if (context->argc != 0) {
    //fprintf(stderr, usage, cmd);
    //return 1;
  //}

  ret = xdo_window_get_active(context->xdo, &window);

  if (ret) {
    fprintf(stderr, "xdo_get_active_window reported an error\n");
  } else {
    window_print(window);
    window_save(context, window);
  }

  return ret;
}

