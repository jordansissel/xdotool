#include "xdo_cmd.h"

int cmd_windowreparent(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  Window wid_source, wid_target;

  int c;
  typedef enum {
    opt_unused, opt_help
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid_source wid_target\n";

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

  if (context->argc != 2) {
    fprintf(stderr, usage, cmd);
    return 1;
   }

  wid_source = (Window)strtol(context->argv[0], NULL, 0);
  wid_target = (Window)strtol(context->argv[1], NULL, 0);

  consume_args(context, 2);

  ret = xdo_window_reparent(context->xdo, wid_source, wid_target);
  if (ret)
    fprintf(stderr, "xdo_window_reparent reported an error\n");

  return ret;
}
