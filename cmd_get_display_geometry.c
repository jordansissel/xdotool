#include "xdo_cmd.h"

int cmd_get_display_geometry(context_t *context) {
  int ret = 0;
  char *cmd = context->argv[0];

  int c;
  int screen = DefaultScreen(context->xdo->xdpy);
  int shell_output = False;

  typedef enum { 
    opt_unused, opt_help, opt_screen, opt_shell
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "screen", required_argument, NULL, opt_screen },
    { "shell", no_argument, NULL, opt_shell },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
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
      case opt_screen:
        screen = atoi(optarg);
        break;
      case opt_shell:
        shell_output = True;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  unsigned int width = 0;
  unsigned int height = 0;
  ret = xdo_get_viewport_dimensions(context->xdo, &width, &height, screen);

  if (shell_output) {
    xdotool_output(context, "WIDTH=%d", width);
    xdotool_output(context, "HEIGHT=%d", height);
  } else {
    xdotool_output(context, "%d %d", width, height);
  }

  return ret;
}
