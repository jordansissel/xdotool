#include "xdo_cmd.h"

struct windowmove {
  Window window;
  int x;
  int y;
  int opsync;
};

static int _windowmove(context_t *context, struct windowmove *windowmove);

int cmd_windowmove(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  struct windowmove windowmove;
  
  windowmove.x = 0;
  windowmove.y = 0;
  windowmove.opsync = 0;
  windowmove.window = CURRENTWINDOW;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_sync
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] [window=%1] x y\n"
    "--sync    - only exit once the window has moved\n";

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
      case opt_sync:
        windowmove.opsync = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  const char *window_arg = "%1";

  if (!window_get_arg(context, 2, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  windowmove.x = (int)strtol(context->argv[0], NULL, 0);
  windowmove.y = (int)strtol(context->argv[1], NULL, 0);

  consume_args(context, 2);  

  window_each(context, window_arg, {
      windowmove.window = window;
      _windowmove(context, &windowmove);
    }); /* window_each(...) */
  return ret;
}

static int _windowmove(context_t *context, struct windowmove *windowmove) {
  int orig_win_x = 0;
  int orig_win_y = 0;
  int ret;

  if (windowmove->opsync) {
    xdo_get_window_location(context->xdo, windowmove->window,
                            &orig_win_x, &orig_win_y, NULL);
    //fprintf(stderr, "%d,%d vs orig:%d,%d\n", windowmove->x, windowmove->y,
            //orig_win_x, orig_win_y);
    if (orig_win_x == windowmove->x && orig_win_y == windowmove->y) {
      /* Break early if we don't need to move the window */
      return 0;
    }
  }

  ret = xdo_window_move(context->xdo, windowmove->window,
                        windowmove->x, windowmove->y);
  if (ret) {
    fprintf(stderr,
            "xdo_window_move reported an error while moving window %ld\n",
            windowmove->window);
  } else {
    if (windowmove->opsync) {
      /* This 'sync' request is stateful (we need to know the original window
       * location to make the decision about 'done' 
       * Some window managers force alignments or otherwise mangle move
       * requests, so we can't just look for the x,y positions exactly.
       * Just look for any change in the window's position. */
      int win_x, win_y;
      xdo_get_window_location(context->xdo, windowmove->window,
                              &win_x, &win_y, NULL);
      /* Permit imprecision to account for window borders and titlebar */
      while (orig_win_x == win_x && orig_win_y == win_y
             && abs(windowmove->x - win_x) > 10
             && abs(windowmove->y - win_y) > 50) {
        xdo_get_window_location(context->xdo, windowmove->window,
                                &win_x, &win_y, NULL);
        usleep(30000);
      }
    }
  }

  return ret;
} 
