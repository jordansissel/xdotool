#include "xdo_cmd.h"

int cmd_windowunmap(context_t *context) {
  int ret = 0;
  Window wid;
  char *cmd = *context->argv;
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
    "Usage: %s [--sync] wid\n"
    "--sync    - only exit once the window has been unmapped (is hidden)\n";

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

  wid = (Window)strtol(context->argv[0], NULL, 0);
  consume_args(context, 1);
  ret = xdo_window_unmap(context->xdo, wid);
  if (ret)
    fprintf(stderr, "xdo_window_unmap reported an error\n");

  if (opsync) {
    xdo_window_wait_for_map_state(context->xdo, wid, IsUnmapped);
  }
  
  return ret;
}

