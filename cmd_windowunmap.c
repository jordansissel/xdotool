#include "xdo_cmd.h"

int cmd_windowunmap(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  const char *window_arg = "%1";
  int consume = 0;
  int opsync;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_sync, opt_verbose
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [--sync] [window=%1]\n"
    "--sync    - only exit once the window has been unmapped (is hidden)\n";

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
      case opt_sync:
        opsync = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);
  
  if (context->argc >= 1 && !is_command(context->argv[0])) {
    consume = 1;
    window_arg = context->argv[0];
  }

  window_each(context, window_arg, {
    ret = xdo_window_unmap(context->xdo, window);
    if (ret) {
      fprintf(stderr, "xdo_window_unmap reported an error\n");
    }

    if (opsync) {
      xdo_window_wait_for_map_state(context->xdo, window, IsUnmapped);
    }
  }); /* window_each(...) */

  consume_args(context, consume);  

  return ret;
}

