#include "xdo_cmd.h"

int cmd_windowactivate(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  int opsync = 0;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_sync
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] wid\n"
    "--sync    - only exit once the window is active (is visible + active)\n";

  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case opt_sync:
        opsync = 1;
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
    ret = xdo_window_activate(context->xdo, window);
    if (ret) {
      fprintf(stderr, "xdo_window_activate on window:%ld reported an error\n",
              window);
      return ret;
    } else {
      if (opsync) {
        xdo_window_wait_for_active(context->xdo, window, 1);
      }
    }
  }); /* window_each(...) */
  consume_args(context, 1);

  return ret;
}

