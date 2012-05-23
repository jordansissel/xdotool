#include "xdo_cmd.h"

/* Added 2007-07-28 - Lee Pumphret */
int cmd_getwindowfocus(context_t *context) {
  int ret = 0;
  int get_toplevel_focus = 1;
  Window window = 0;
  char *cmd = context->argv[0];

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 'f' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [-f]\n"
    "-f     - Report the window with focus even if we don't think it is a \n"
    "         top-level window. The default is to find the top-level window\n"
    "         that has focus.\n";
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+fh",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 'f':
        get_toplevel_focus = 0;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  //if (context->argc > 0) {
    //fprintf(stderr, usage, cmd);
    //return 1;
  //}

  if (get_toplevel_focus) {
    ret = xdo_get_focused_window_sane(context->xdo, &window);
  } else {
    ret = xdo_get_focused_window(context->xdo, &window);
  }

  if (ret) {
    fprintf(stderr, "xdo_focus_window reported an error\n");
  } else { 
    /* only print if we're the last command */
    if (context->argc == 0) {
      window_print(window);
    }
    window_save(context, window);
  }

  return ret;
}

