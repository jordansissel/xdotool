#include "xdo_cmd.h"

int cmd_set_desktop(context_t *context) {
  char *cmd = *context->argv;
  long desktop;
  int relative = False;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_relative
  } optlist_t;

  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "relative", no_argument, NULL, opt_relative },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s desktop\n"
    "--relative    - Move relative to the current desktop. Negative values OK\n";
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
      case opt_relative:
        relative = True;
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

  desktop = strtol(context->argv[0], NULL, 0);

  consume_args(context, 1);

  if (relative == True) {
    long cur_desktop = 0, ndesktops = 0;
    xdo_get_current_desktop(context->xdo, &cur_desktop);
    xdo_get_number_of_desktops(context->xdo, &ndesktops);

    desktop = (desktop + cur_desktop) % ndesktops;

    /* negative mod doesn't result in a positive number. Fix that. */
    if (desktop < 0) 
      desktop += ndesktops;
  }

  return xdo_set_current_desktop(context->xdo, desktop);
}

