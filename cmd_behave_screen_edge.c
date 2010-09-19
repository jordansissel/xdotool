#include "xdo_cmd.h"
#include <string.h>

/* TODO(sissel): Implement XRANDR so we can detect when screen sizes change */
/* So we can invoke xdotool from within this command */
extern int context_execute(context_t *context);

typedef enum {
  none, left, top_left, top, top_right, right,
  bottom_right, bottom, bottom_left,
} edge_or_corner;

int is_edge_or_corner(const xdo_t *xdo, const edge_or_corner what, 
                      const Window window, const unsigned int x,
                      const unsigned int y);

int cmd_behave_screen_edge(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  xdo_search_t search;
  Window *windowlist;
  int nwindows;

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

  const char *edge_or_corner_spec = context->argv[0];
  consume_args(context, 1);

  /* The remainder of args are supposed to be what to run when the edge
   * or corner is hit */

  /* TODO(sissel): Refactor this into libxdo */
  memset(&search, 0, sizeof(xdo_search_t));
  search.max_depth = -1;
  search.require = SEARCH_ANY;
  search.searchmask = SEARCH_NAME;
  search.winname = "^"; /* Match anything */
  xdo_window_search(context->xdo, &search, &windowlist, &nwindows);
  int i;
  for (i = 0; i < nwindows; i++) {
    XSelectInput(context->xdo->xdpy, windowlist[i], PointerMotionMask | SubstructureNotifyMask);
  }

  edge_or_corner want;
  if (!strcmp(edge_or_corner_spec, "left")) { /* left */
    want = left;
  } else if (!strcmp(edge_or_corner_spec, "top_left")) {
    want = top_left;
  } else if (!strcmp(edge_or_corner_spec, "top")) {
    want = top;
  } else if (!strcmp(edge_or_corner_spec, "top_right")) {
    want = top_right;
  } else if (!strcmp(edge_or_corner_spec, "right")) {
    want = right;
  } else if (!strcmp(edge_or_corner_spec, "bottom_right")) {
    want = bottom_right;
  } else if (!strcmp(edge_or_corner_spec, "bottom")) {
    want = bottom;
  } else if (!strcmp(edge_or_corner_spec, "bottom_left")) {
    want = bottom_left;
  } else {
    fprintf(stderr, "Invalid edge or corner, '%s'\n", edge_or_corner_spec);
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  } 

  int need_new_context = True;
  context_t *tmpcontext = NULL;
  edge_or_corner state = none;
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
      case CreateNotify:
        XSelectInput(context->xdo->xdpy, e.xcreatewindow.window, PointerMotionMask | SubstructureNotifyMask);
        break;
      case MotionNotify:
        //printf("%ld: %d,%d\n", e.xmotion.subwindow ? e.xmotion.subwindow : e.xmotion.window,
               //e.xmotion.x_root, e.xmotion.y_root);

        /* TODO(sissel): Make a dispatch table for this? */
        if (is_edge_or_corner(context->xdo, want, e.xmotion.root,
                              e.xmotion.x_root, e.xmotion.y_root)) {
          if (state == none) {
            state = want;
            trigger = True;
          }
        } else {
          if (state != none) {
            state = none;
          }
        }

        if (trigger == True) {
          ret = context_execute(tmpcontext);
          need_new_context = True;
          trigger = False;
        }
        break;
      case DestroyNotify:
      case UnmapNotify:
      case MapNotify:
      case ConfigureNotify:
      case ClientMessage:
      case ReparentNotify:
        /* Ignore */
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
} /* int cmd_behave_screen_edge */

int is_edge_or_corner(const xdo_t *xdo, const edge_or_corner what, 
                      const Window window, const unsigned int x,
                      const unsigned int y) {
  unsigned int width;
  unsigned int height;
  xdo_get_window_size(xdo, window, &width, &height);

  unsigned int x_max = width - 1;
  unsigned int y_max = height - 1;

  switch (what) {
    case left: return (x == 0); break;
    case top_left: return (x == 0 && y == 0); break;
    case top: return (y == 0); break;
    case top_right: return (x == x_max && y == 0); break;
    case right: return (x == x_max); break;
    case bottom_right: return (x == x_max && y == y_max); break;
    case bottom: return (y == y_max); break;
    case bottom_left: return (x == 0 && y == y_max); break;
    case none: return False; break;
  }

  return False;
} /* int is_edge_or_corner */
