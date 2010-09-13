#include "xdo_cmd.h"
#include <string.h>

struct events {
  const char * const name;
  int mask;
} events[] = {
  { "mouse-enter", EnterWindowMask },
  { "mouse-leave", LeaveWindowMask },
  { "focus", FocusChangeMask },
  { "blur", FocusChangeMask },
  { "mouse-click", ButtonReleaseMask },
  { NULL, 0 },
};

/* So we can invoke xdotool from within this command */
extern int context_execute(context_t *context);

int cmd_behave(context_t *context) {
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
    "Usage: %s window event action [args...]\n"
    "The event is a window event, such as mouse-enter, resize, etc.\n"
    "The action is any valid xdotool command (chains OK here)\n"
    "\n"
    "Events: \n"
    "  mouse-enter      - When the mouse moves into the window\n"
    "  mouse-leave      - When the mouse leaves a window\n"
    "  mouse-click      - Fired when the mouse button is released\n"
    "  focus            - When the window gets focus\n"
    "  blur             - When the window loses focus\n";

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

  if (context->argc < 3) {
    fprintf(stderr, "Invalid number of arguments (minimum is 3)\n");
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  /* Can't use consume_args since 'behave' eats the rest of the line. */
  /* TODO(sissel): make it work. */
  const char *window_arg = context->argv[0];
  consume_args(context, 1);

  const char *event = context->argv[0];
  consume_args(context, 1);

  /* The remainder of args are supposed to be what to run on the action */

  long selectmask = 0;
  int i;
  for (i = 0; events[i].name != NULL; i++) {
    //printf("%s vs %s\n", events[i].name, event);
    if (!strcmp(events[i].name, event)) {
      selectmask |= events[i].mask;
    }
  }

  if (selectmask == 0) {
    fprintf(stderr, "Unknown event '%s'\n", event);
    return EXIT_FAILURE;
  }

  window_each(context, window_arg, {
    ret = XSelectInput(context->xdo->xdpy, window, selectmask);
    if (ret != True) {
      fprintf(stderr, "XSelectInput reported an error\n");
    }
  }); /* window_each(...) */

  while (True) {
    XEvent e;
    XNextEvent(context->xdo->xdpy, &e);

    context_t *tmpcontext = calloc(1, sizeof(context_t));
    memcpy(tmpcontext, context, sizeof(context_t));

    tmpcontext->nwindows = 1;
    Window hover; /* for LeaveNotify */
    switch (e.type) {
      case LeaveNotify:
        /* LeaveNotify is confusing.
         * It is sometimes fired when you are actually entering the window
         * especially at the screen edges. not sure why or what causes it.
         * Work around: Query the window the mouse is over. If it is not
         * us, then we can fire leave.  */

        /* allow some time to pass to let the mouse really leave if we are on our way out */
        /* TODO(sissel): allow this delay to be tunable */
        usleep(100000); /* 100ms */
        xdo_mousewindow(context->xdo, &hover);
        if (hover == e.xcrossing.window) {
          //printf("Ignoring Leave, we're still in the window\n");
          break;
        }
        //printf("Window: %ld\n", e.xcrossing.window);
        //printf("Hover: %ld\n", hover);

        /* fall through */
      case EnterNotify:
        tmpcontext->windows = &(e.xcrossing.window);
        ret = context_execute(tmpcontext);
        break;
      case FocusIn:
      case FocusOut:
        tmpcontext->windows = &(e.xfocus.window);
        ret = context_execute(tmpcontext);
        break;
      case ButtonRelease:
        tmpcontext->windows = &(e.xbutton.window);
        ret = context_execute(tmpcontext);
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

