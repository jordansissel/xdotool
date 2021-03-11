#include "xdo_cmd.h"
#include <string.h>

int cmd_windowsize(context_t *context) {
  int ret = 0;
  unsigned int width, height;
  const char *warg, *harg;
  int parse_flags;
  int c;
  int opsync = 0;

  int use_hints = 0;
  typedef enum {
    opt_unused, opt_help, opt_usehints, opt_sync
  } optlist_t;
  struct option longopts[] = {
    { "usehints", 0, NULL, opt_usehints },
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };

  int size_flags = 0;
  char *cmd = *context->argv;
  int option_index;
  static const char *usage =
            "Usage: %s [--sync] [--usehints] [window=%1] width height\n"
            HELP_SEE_WINDOW_STACK
            "--usehints  - Use window sizing hints (like font size in terminals)\n"
            "--sync      - only exit once the window has resized\n";


  while ((c = getopt_long_only(context->argc, context->argv, "+uh",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
      case 'u':
      case opt_usehints:
        use_hints = 1;
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

  const char *window_arg = "%1";

  if (!window_get_arg(context, 2, 0, &window_arg)) {
    fprintf(stderr, "Invalid argument count, got %d, expected %d\n", 
            3, context->argc);
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  warg = context->argv[0];
  harg = context->argv[1];
  consume_args(context, 2);

  unsigned int original_w, original_h;

  window_each(context, window_arg, {
    xdo_get_window_size(context->xdo, window, &original_w, &original_h);
    width = original_w;
    height = original_h;
    xdo_get_xy(context->xdo, window, warg, harg, &width, &height, &parse_flags);

    if (use_hints) {
      if (!(parse_flags & GETXY_PERCENT_Y)) {
        size_flags |= SIZE_USEHINTS_Y;
      }
      if (!(parse_flags & GETXY_PERCENT_X)) {
        size_flags |= SIZE_USEHINTS_X;
      }
    }

    if (opsync) {
      unsigned int w = width;
      unsigned int h = height;
      if (size_flags & SIZE_USEHINTS_X) {
        xdo_translate_window_with_sizehint(context->xdo, window, w, h, &w, NULL);
      }
      if (size_flags & SIZE_USEHINTS_Y) {
        xdo_translate_window_with_sizehint(context->xdo, window, w, h, NULL, &h);
      }

      if (original_w == w && original_h == h) {
        /* Skip, this window doesn't need to move. */
        break;
      }
    }

    ret = xdo_set_window_size(context->xdo, window, width, height, size_flags);
    if (ret) {
      fprintf(stderr, "xdo_set_window_size on window:%ld reported an error\n",
              window);
      return ret;
    }
    if (opsync) {
      //xdo_wait_for_window_size(context->xdo, window, width, height, 0,
                               //SIZE_TO);
      xdo_wait_for_window_size(context->xdo, window, original_w, original_h, 0,
                               SIZE_FROM);
    }
  }); /* window_each(...) */

  return ret;
}

