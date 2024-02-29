#include "xdo_cmd.h"

int cmd_getmouselocation(context_t *context) {
  int rx, ry, screen_num;
  Window window_under_cursor;
  int ret;
  char *cmd = context->argv[0];
  char *window_arg = NULL;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "shell", no_argument, NULL, 's' },
    { "prefix", required_argument, NULL, 'p' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [--shell] [--prefix <STR>]\n"
    "--window <windowid>    - get mouse location relative to window\n"
    "--shell                - output shell variables for use with eval\n"
    "--prefix STR           - use prefix for shell variables names (max 16 chars)\n";
  int option_index;
  int output_shell = 0;
  char out_prefix[17] = {'\0'};

  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 'w':
        window_arg = strdup(optarg);
        break;
      case 's':
        output_shell = 1;
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

  ret = xdo_get_mouse_location2(context->xdo, &rx, &ry, &screen_num, &window_under_cursor);

  int x = rx;
  int y = ry;

  int wx = COORDINATE_NONE;
  int wy = COORDINATE_NONE;
  if (window_arg) {
    window_each(context, window_arg, {
      xdo_translate_coordinates_to_window(context->xdo, window, rx, ry, &wx, &wy);
      x = wx;
      y = wy;
    }); /* window_each(...) */
  }

  if (output_shell) {
    xdotool_output(context, "%sX=%d", out_prefix, rx);
    xdotool_output(context, "%sY=%d", out_prefix, ry);
    if (window_arg) {
      xdotool_output(context, "%sWX=%d", out_prefix, wx);
      xdotool_output(context, "%sWY=%d", out_prefix, wy);
    }
    xdotool_output(context, "%sSCREEN=%d", out_prefix, screen_num);
    xdotool_output(context, "%sWINDOW=%d", out_prefix, window_under_cursor);
  } else {
    /* only print if we're the last command */
    if (context->argc == 0) {
      xdotool_output(context, "x:%d y:%d screen:%d window:%ld", x, y, screen_num, window_under_cursor);
    }
    window_save(context, window_under_cursor);
  }

  free(window_arg);

  return ret;
}

