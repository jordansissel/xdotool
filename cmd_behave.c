#include "xdo_cmd.h"
#include <string.h>

struct events {
  const char * const name;
  int mask;
} events[] = {
  { "mouse-enter", EnterWindowMask },
  { "mouse-leave", LeaveWindowMask },
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
    "Usage: %s [window=%1] event action [args...]\n"
    "The event is a window event, such as mouse-enter, resize, etc.\n"
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

  /* Can't use consume_args since 'behave' eats the rest of the line. */
  /* TODO(sissel): make it work. */
  const char *window_arg = context->argv[0];
  consume_args(context, 1);

  const char *event = context->argv[0];
  consume_args(context, 1);

  //printf("Window: %s\n", window_arg);
  //printf("Event: %s\n", event);
  int i = 0;
  //printf("remaining %d: ", context->argc);
  for (i = 0; i < context->argc; i++) {
    printf("%s ", context->argv[i]);
  }
  printf("\n");

  /* The remainder of args are supposed to be what to run on the action */
  
  /* Actions:
   * - mouse-enter
   * - mouse-leave
   * - focus
   * - unfocus
   * - 
   * ...
   */

  long selectmask = 0;
  for (i = 0; events[i].name != NULL; i++) {
    printf("Checking: %s vs %s\n", events[i].name, event);
    if (!strcmp(events[i].name, event)) {
      selectmask |= events[i].mask;
    }
  }

  if (selectmask == 0) {
    fprintf(stderr, "No events selected. Aborting.\n");
    return EXIT_FAILURE;
  }

  window_each(context, window_arg, {
    ret = XSelectInput(context->xdo->xdpy, window, selectmask);
    if (ret != True) {
      fprintf(stderr, "XSelectInput  reported an error\n");
    }
  }); /* window_each(...) */

  /* Make a new char **argv... */
  //const char **argv = calloc(context->argc + 1, sizeof(char *));
  //argv[0] = context->prog;
  //memcpy(argv + 1, context->argv, context->argc * sizeof(char *));

  while (True) {
    XEvent e;
    XNextEvent(context->xdo->xdpy, &e);

    context_t *tmpcontext = calloc(1, sizeof(context_t));
    memcpy(tmpcontext, context, sizeof(context_t));

    switch (e.type) {
      case EnterNotify:
      case LeaveNotify:
        tmpcontext->windows = &(e.xcrossing.window);
        tmpcontext->nwindows = 1;
        printf("Window: %ld\n", tmpcontext->windows[0]);
        printf("Exec: %s %s %s %s\n",
               tmpcontext->argv[0],
               tmpcontext->argv[1],
               tmpcontext->argv[2],
               tmpcontext->argv[3]);
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

