#include "xdo_cmd.h"
#include <string.h>
#include <sys/select.h> /* for select */
#include <time.h> /* for clock_gettime */
#include <sys/time.h> /* for timeradd and friends */

#if defined(MISSING_CLOCK_GETTIME)
/* http://code.google.com/p/semicomplete/issues/detail?id=37
 * OS X doesn't support clock_gettime (in at least OSX <= 10.11) */
#  include "patch_clock_gettime.h"
#endif

/* TODO(sissel): Refactor the madness.
 * The event look, delay and quiesce state handling, etc, are all pretty
 * intermingled. This needs a serious refactor into separate functions and
 * a struct to hold state, etc.
 */

/* TODO(sissel): Implement XRANDR so we can detect when screen sizes change?
 *
 * Then again, since we always query the size of the window (root) that
 * the mouse is in, maybe it doesn't matter.
 *
 * Possible improvements to this could be to select ConfigureNotify so
 * we can be told of window size changes rather than querying the root
 * window size every time. 
 */
extern int context_execute(context_t *context);

typedef enum {
  none, left, top_left, top, top_right, right,
  bottom_right, bottom, bottom_left,
} edge_or_corner;

int is_edge_or_corner(const xdo_t *xdo, const edge_or_corner what, 
                      const Window window, const unsigned int x,
                      const unsigned int y);
int ignore_error(Display *dpy, XErrorEvent *xerr);

int cmd_behave_screen_edge(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  xdo_search_t search;
  Window *windowlist;
  unsigned int nwindows;
  useconds_t delay = 0;
  useconds_t quiesce = 2000000 /* 2 second default quiesce */;
  enum { quiesce_start, quiesce_ing, quiesce_done } quiesce_state = quiesce_start;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_delay, opt_quiesce
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "delay", required_argument, NULL, opt_delay },
    { "quiesce", required_argument, NULL, opt_quiesce },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] edge-or-corner action [args...]\n"
    "--delay MILLISECONDS     - delay before activating. During this time,\n"
    "        your mouse must stay in the area selected (corner or edge)\n"
    "        otherwise this timer will reset. Default is no delay (0).\n"
    "--quiesce MILLISECONDS   - quiet time period after activating that no\n"
    "        new activation will occur. This helps prevent accidental\n"
    "        re-activation immediately after an event. Default is 2000 (2\n"
    "        seconds)."
    "\n"
    "edge-or-corner can be any of:\n"
    "  Edges: left, top, right, bottom\n"
    "  Corners: top-left, top-right, bottom-left, bottom-right\n"
    "The action is any valid xdotool command (chains OK here)\n";

  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "+hdq",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case 'd':
      case opt_delay:
        delay = atoi(optarg) * 1000; /* convert ms to usec */
        /* TODO(sissel): Do validation */
        break;
      case 'q':
      case opt_quiesce:
        quiesce = atoi(optarg) * 1000; /* convert ms to usec */
        /* TODO(sissel): Do validation */
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
  xdo_search_windows(context->xdo, &search, &windowlist, &nwindows);
  unsigned int i;
  for (i = 0; i < nwindows; i++) {
    XSelectInput(context->xdo->xdpy, windowlist[i], PointerMotionMask | SubstructureNotifyMask);
  }

  edge_or_corner want;
  if (!strcmp(edge_or_corner_spec, "left")) {
    want = left;
  } else if (!strcmp(edge_or_corner_spec, "top-left")) {
    want = top_left;
  } else if (!strcmp(edge_or_corner_spec, "top")) {
    want = top;
  } else if (!strcmp(edge_or_corner_spec, "top-right")) {
    want = top_right;
  } else if (!strcmp(edge_or_corner_spec, "right")) {
    want = right;
  } else if (!strcmp(edge_or_corner_spec, "bottom-right")) {
    want = bottom_right;
  } else if (!strcmp(edge_or_corner_spec, "bottom")) {
    want = bottom;
  } else if (!strcmp(edge_or_corner_spec, "bottom-left")) {
    want = bottom_left;
  } else {
    fprintf(stderr, "Invalid edge or corner, '%s'\n", edge_or_corner_spec);
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  } 

  int need_new_context = True;
  context_t *tmpcontext = NULL;
  edge_or_corner state = none;
  struct timeval sleeptime = {-1,0};
  struct timeval triggertime = {0,0};
  struct timeval delaytime = { delay / 1000000, delay % 1000000 };
  struct timeval quiescetime = { quiesce / 1000000, quiesce % 1000000 };
  struct timeval quiesceuntil = {0,0};
  //printf("Delay time: %ld.%ld\n", delaytime.tv_sec, delaytime.tv_usec);
  struct timeval tmptime = {0,0};
  struct timespec now = {0,0};

  fd_set fdset;
  int xfd = XConnectionNumber(context->xdo->xdpy);
  FD_ZERO(&fdset);
  FD_SET(xfd, &fdset);

  /* Flush any pending requests before we start looping */
  XFlush(context->xdo->xdpy);

  while (True) {
    XEvent e;
    e.type = 0; /* clear event */

    fd_set fdset_copy = fdset; 
    int ready = 0;

    /* TODO(sissel): use select, not XNextEvent */
    /* Take tv_sec value of '-1' to mean block-forever */

    ready = select(xfd + 1, &fdset_copy, NULL, NULL,
                   (sleeptime.tv_sec < 0 ? NULL : &sleeptime));
    sleeptime.tv_sec = -1; /* default to block forever */

    int trigger = False;
    if (ready) {
      XNextEvent(context->xdo->xdpy, &e);
    } else {
      /* timeout */
      printf("Timeout\n");
      if (quiesce_state == quiesce_ing) {
        quiesce_state = quiesce_done;
      } else if (state == want) {
        trigger = True;
      }
    }

    /* Only create a context copy if we need one */
    if (need_new_context) {
      tmpcontext = calloc(1, sizeof(context_t));
      memcpy(tmpcontext, context, sizeof(context_t));
    }

    int (*old_error_handler)(Display *dpy, XErrorEvent *xerr);
    switch (e.type) {
      case CreateNotify:
        /* Ignore selection errors. Errors can occur if we try to XSelectInput
         * after a window has been destroyed */
        old_error_handler = XSetErrorHandler(ignore_error);
        XSelectInput(context->xdo->xdpy, e.xcreatewindow.window, PointerMotionMask | SubstructureNotifyMask);
        XSync(context->xdo->xdpy, False);
        XSetErrorHandler(old_error_handler);
        break;
      case MotionNotify:
        /* TODO(sissel): Put this in a function */
        //printf("%ld: %d,%d\n", e.xmotion.subwindow ? e.xmotion.subwindow : e.xmotion.window,
               //e.xmotion.x_root, e.xmotion.y_root);

        /* TODO(sissel): Make a dispatch table for this? */
        if (is_edge_or_corner(context->xdo, want, e.xmotion.root,
                              e.xmotion.x_root, e.xmotion.y_root)) {
          /* If we get here, then we're in the edge/corner we wanted */
          if (state == none) {
            state = want;

            if (delay > 0) {
              /* Calculate when we should trigger */
              clock_gettime(CLOCK_MONOTONIC, &now);
              tmptime.tv_sec = now.tv_sec;
              tmptime.tv_usec = now.tv_nsec / 1000;
              timeradd(&tmptime, &delaytime, &triggertime);
              /* Set select() to sleep on our delay */
              memcpy(&sleeptime, &delaytime, sizeof(struct timeval));
            } else {
              trigger = True;
            }
          } else { /* else, we are still in wanted edge/corner state */

            /* Only care if we have a trigger delay */
            if (timerisset(&triggertime)) {
              /* Check if current time exceeds trigger time */
              clock_gettime(CLOCK_MONOTONIC, &now);
              tmptime.tv_sec = now.tv_sec;
              tmptime.tv_usec = now.tv_nsec / 1000;
              if (timercmp(&tmptime, &triggertime, >=)) {
                trigger = True;
              } else {
                /* Not time yet, so next sleep should be the
                 * remainder of time left */
                timersub(&triggertime, &tmptime, &sleeptime);
              }
            } /* if delay > 0 && timerisset(&triggertime) */
          }
        } else { /* else, we are not in an edge/corner */
          if (state != none) {
            state = none;
            delaytime.tv_usec = -1;
          }
        }
        break;
      case DestroyNotify:
      case UnmapNotify:
      case MapNotify:
      case ConfigureNotify:
      case ClientMessage:
      case ReparentNotify:
      case 0: /* Special "non event" internal to xdotool */
        /* Ignore */
        break;
      default:
        printf("Unexpected event: %d\n", e.type);
        break;
    } /* X11 event handling */

    if (trigger == True) {
      //printf("Quiesce: %d / %d == %d?\n", quiesce, quiesce_state, quiesce_ing);
      if (quiesce > 0 && quiesce_state == quiesce_ing) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        tmptime.tv_sec = now.tv_sec;
        tmptime.tv_usec = now.tv_nsec / 1000;
        //printf("Start waiting until %ld.%ld (now: %ld.%ld) due to quiesce\n",
               //quiesceuntil.tv_sec, quiesceuntil.tv_usec,
               //tmptime.tv_sec, tmptime.tv_usec);
        if (timercmp(&quiesceuntil, &tmptime, >)) {
          /* Don't trigger yet. Wait until quiesceuntil time. */
          timersub(&quiesceuntil, &tmptime, &sleeptime);
          //printf("Waiting for %ld.%ld due to quiesce\n", sleeptime.tv_sec,
                 //sleeptime.tv_usec);
          continue;
        } /* if quiesceuntil has not passed yet */
        quiesce_state = quiesce_done;
      } /* if quiesce */

      ret = context_execute(tmpcontext);
      need_new_context = True;
      trigger = False;
      timerclear(&triggertime);
      if (quiesce > 0) {
        /* TODO(sissel): refactor clock_gettime calls */
        clock_gettime(CLOCK_MONOTONIC, &now); 
        quiesceuntil.tv_sec = now.tv_sec;
        quiesceuntil.tv_usec = now.tv_nsec / 1000;
        timeradd(&quiesceuntil, &quiescetime, &quiesceuntil);
        quiesce_state = quiesce_ing;
        //printf("Start waiting for %ld.%ld due to quiesce\n", quiesceuntil.tv_sec,
               //quiesceuntil.tv_usec);
      } /* if quiesce */
    } /* if trigger == True */

    if (tmpcontext != NULL) {
      free(tmpcontext);
    }

    if (ret != XDO_SUCCESS) {
      printf("Command failed.\n");
    }
  } /* while True */
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

int ignore_error(Display *dpy, XErrorEvent *xerr) {
  dpy = dpy; /* Just use these to avoid warnings. */
  xerr = xerr;
  return 0;
}

