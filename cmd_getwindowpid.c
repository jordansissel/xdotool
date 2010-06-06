#include "xdo_cmd.h"

int cmd_getwindowpid(context_t *context) {
  Window wid = 0;
  int pid;
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window id>\n";
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

  if (context->argc < 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  wid = (Window)strtol(context->argv[0], NULL, 0);
  pid = xdo_window_get_pid(context->xdo, wid);

  consume_args(context, 1);
  if (pid == 0) {
    fprintf(stderr, "window %ld has no pid associated with it.\n", wid);
    return EXIT_FAILURE;
  } else {
    printf("%d\n", pid);
    return EXIT_SUCCESS;
  }
}

