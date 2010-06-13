#include "xdo_cmd.h"

int cmd_set_num_desktops(context_t *context) {
  char *cmd = *context->argv;
  long ndesktops;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s num_desktops\n";
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

  ndesktops = strtol(context->argv[0], NULL, 0);

  consume_args(context, 1);
  return xdo_set_number_of_desktops(context->xdo, ndesktops);
}

