#include "xdo_cmd.h"

int cmd_windowreparent(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  const char *window_arg = "%1";

  int c;
  typedef enum {
    opt_unused, opt_help
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s [window_source=%1] window_destination\n";

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

  if (!window_get_arg(context, 1, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  /* Permit using WINDOW STACK notation for the destination window, too */
  Window *destwindows = NULL;
  int ndestwindows = 0;
  window_list(context, context->argv[0], &destwindows, &ndestwindows, False); \

  if (ndestwindows > 1) {
    fprintf(stderr, "It doesn't make sense to have multiple destinations as the "
            "new parent window. Your destination selection '%s' resulted in %d "
            "windows.", context->argv[0], ndestwindows);
    return EXIT_FAILURE;
  }
  Window destination = destwindows[0];

  consume_args(context, 1);

  window_each(context, window_arg, {
    //printf("Reparenting %ld -> %ld\n", window, destination);
    ret = xdo_reparent_window(context->xdo, window, destination);
    if (ret) {
      fprintf(stderr, "xdo_reparent_window reported an error on for "
              "src=%ld, dest=%ld\n", window, destination);
    }
  }); /* window_each(...) */

  if (ret)
    fprintf(stderr, "xdo_reparent_window reported an error\n");

  return ret;
}
