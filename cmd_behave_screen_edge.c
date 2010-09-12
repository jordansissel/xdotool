
#include "xdo_cmd.h"
#include <string.h>

/* So we can invoke xdotool from within this command */
extern int context_execute(context_t *context);

typedef enum {
  none, left, top_left, top, top_right, right,
  bottom_right, bottom, bottom_left,
} current_edge_or_corner ;

int cmd_behave_screen_edge(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;

  int c;
  typedef enum {
    opt_unused, opt_help
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] edge-or-corner action [args...]\n"
    "\n"
    "edge-or-corner can be any of (to specify multiple, separate by comma):\n"
    "  Edges: left, top, right, bottom\n"
    "  Corners: top-left, top-right, bottom-left, bottom-right\n"
    "The action is any valid xdotool command (chains OK here)\n";

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
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc < 2) {
    fprintf(stderr, "Invalid number of arguments (minimum is 2)\n");
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  /* Can't use consume_args since this command uses the rest of the line. */
  const char *edge_or_corner_spec = context->argv[0];
  consume_args(context, 1);

  /* The remainder of args are supposed to be what to run when the edge
   * or corner is hit */

  /* TODO(sissel): Refactor this into libxdo */

  long selectmask = PointerMotionMask;
  int screencount = ScreenCount(context->xdo->xdpy);
  int i = 0;
  for (i = 0; i < screencount; i++) {
    Screen *screen = ScreenOfDisplay(context->xdo->xdpy, i);
    Window root = RootWindowOfScreen(screen);
    XSelectInput(context->xdo->xdpy, root, selectmask);
  }

  int need_new_context = True;
  context_t *tmpcontext = NULL;
  current_edge_or_corner state = none;
  while (True) {
    XEvent e;
    XNextEvent(context->xdo->xdpy, &e);

    /* Only create a context copy if we need one */
    if (need_new_context) {
      tmpcontext = calloc(1, sizeof(context_t));
      memcpy(tmpcontext, context, sizeof(context_t));
    }

    int trigger = False;
    switch (e.type) {
      case MotionNotify:
        //printf("%d,%d\n", e.xmotion.x_root, e.xmotion.y_root);
        /* TODO(sissel): Make a dispatch table for this */
        if (e.xmotion.x_root == 0 && !strcmp(edge_or_corner_spec, "left")) { /* left */
          if (state == none) {
            state = left;
            trigger = True;
          }
        } else if (e.xmotion.y_root == 0 && !strcmp(edge_or_corner_spec, "top")) { /* top */
          if (state == none) {
            state = top;
            trigger = True;
          }
        } else {
          if (state != none) {
            printf("Resetting state\n");
            state = none;
          }
        }
        if (trigger == True) {
          ret = context_execute(tmpcontext);
          need_new_context = True;
          trigger = False;
        }
        break;
      default:
        printf("Unexpected event: %d\n", e.type);
        break;
    }

    if (ret != XDO_SUCCESS) {
      printf("Command failed.\n");
    }
  }
  return ret;
}

