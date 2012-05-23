#include "xdo_cmd.h"

int cmd_getwindowpid(context_t *context) {
  int pid;
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [window=%1]\n"
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

  const char *window_arg = "%1";
  if (!window_get_arg(context, 0, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  window_each(context, window_arg, {
    pid = xdo_get_pid_window(context->xdo, window);
    if (pid == 0) {
      /* TODO(sissel): probably shouldn't exit failure when iterating over
       * a list of windows. What should we do? */
      fprintf(stderr, "window %ld has no pid associated with it.\n", window);
      return EXIT_FAILURE;
    } else {
      xdotool_output(context, "%d", pid);
    }
  }); /* window_each(...) */
  return EXIT_SUCCESS;
}

