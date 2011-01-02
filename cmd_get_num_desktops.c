#include "xdo_cmd.h"

int cmd_get_num_desktops(context_t *context) {
  int ret = 0;
  char *cmd = context->argv[0];
  long ndesktops = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
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

  ret = xdo_get_number_of_desktops(context->xdo, &ndesktops);

  xdotool_output(context, "%ld", ndesktops);
  return ret;
}
