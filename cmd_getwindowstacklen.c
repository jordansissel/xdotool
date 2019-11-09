#include "xdo_cmd.h"

int cmd_getwindowstacklen(context_t *context) {
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s \n"
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
  xdotool_output(context, "stack_length:%i", context->nwindows);

  return context->nwindows? EXIT_SUCCESS : EXIT_FAILURE;
}

