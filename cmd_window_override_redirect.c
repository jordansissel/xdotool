#include "xdo_cmd.h"

int cmd_window_override_redirect(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  const char *window_arg = "%1";
  int opsync = 0;

  int c;
  enum { opt_unused, opt_help, opt_sync };
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] <0 or 1>\n"
    HELP_SEE_WINDOW_STACK;

  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
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
  printf("Args: %d\n", context->argc);

  if (context->argc != 1) {
    fprintf(stderr, "You specified the wrong number of args.\n");
    fprintf(stderr, usage, cmd);
    return 1;
  }

  int val = atoi(context->argv[0]);
  consume_args(context, 1);
  window_each(context, window_arg, {
    ret = xdo_set_window_override_redirect(context->xdo, window, val);
    if (ret) {
      fprintf(stderr, "xdo_map_window reported an error\n");
    }
  }); /* window_each(...) */

  return ret;
}
