#include "xdo_cmd.h"

int cmd_getwindowgeometry(context_t *context) {
  char *cmd = context->argv[0];
  int x, y;
  Screen *screen;
  unsigned int width, height;

  int shell_output = False;
  char out_prefix[17] = {'\0'};

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "shell", no_argument, NULL, 's' },
    { "prefix", required_argument, NULL, 'p' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [window=%1] [--shell] [--prefix <STR>]\n"
    "--shell      - output shell variables for use with eval\n"
    "--prefix STR - use prefix for shell variables names (max 16 chars) \n"
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
      case 's':
        shell_output = True;
        break;
      case 'p':
        strncpy(out_prefix, optarg, sizeof(out_prefix)-1);
        out_prefix[ sizeof(out_prefix)-1 ] = '\0'; //just in case
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
    int ret = 0;
    ret = xdo_get_window_size(context->xdo, window, &width, &height);
    if (ret != XDO_SUCCESS) {
      fprintf(stderr, "window %ld - failed to get height/width?\n", window);
    }

    ret = xdo_get_window_location(context->xdo, window, &x, &y, &screen);
    if (ret != XDO_SUCCESS) {
      fprintf(stderr, "window %ld - failed to get location?\n", window);
    }

    if (shell_output) {
      xdotool_output(context, "%sWINDOW=%ld", out_prefix, window);
      xdotool_output(context, "%sX=%d", out_prefix, x);
      xdotool_output(context, "%sY=%d", out_prefix, y);
      xdotool_output(context, "%sWIDTH=%u", out_prefix, width);
      xdotool_output(context, "%sHEIGHT=%u", out_prefix, height);
      xdotool_output(context, "%sSCREEN=%d", out_prefix, XScreenNumberOfScreen(screen));
    } else {
      xdotool_output(context, "Window %ld", window);
      xdotool_output(context, "  Position: %d,%d (screen: %d)", x, y,
                     XScreenNumberOfScreen(screen));
      xdotool_output(context, "  Geometry: %ux%u", width, height);
    }
  }); /* window_each(...) */
  return EXIT_SUCCESS;
}

