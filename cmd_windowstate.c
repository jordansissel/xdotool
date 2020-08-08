#include <ctype.h>
#include <errno.h>
#include "xdo_cmd.h"

static char *parse_property(const char *arg_property) {
  const char *prefix = "_NET_WM_STATE_";
  const size_t len_prefix = strlen(prefix);
  char *property;
  char *pdst;
  const char *psrc;

  property = (char *) malloc(len_prefix + strlen(arg_property) + 1);
  if (!property) {
    return NULL;
  }
  strcpy(property, prefix);
  pdst = property + len_prefix;
  psrc = arg_property;
  while (*psrc) {
    *pdst++ = (char) toupper((int) *psrc++);
  }
  *pdst = 0;
  return property;
}

int cmd_windowstate(context_t *context) {
  int ret = 0;
  int has_error = 0;

  char *cmd = *context->argv;
  int c;
  const char *window_arg = "%1";

  unsigned long action = (unsigned long) -1;
  const char *arg_property = NULL;
  char *property;

  static struct option longopts[] = {
          {"add",    required_argument, NULL, 'a'},
          {"remove", required_argument, NULL, 'r'},
          {"toggle", required_argument, NULL, 't'},
          {"help",   no_argument,       NULL, 'h'},
          {0, 0, 0,                           0},
  };
  int option_index = 0;
  static const char *usage =
          "Usage: %s [options] [window=%1]\n"
                  HELP_SEE_WINDOW_STACK
                  "--add property  - add a property\n"
                  "--remove property - remove a property\n"
                  "--toggle property - toggle a property\n"
                  "property can be one of \n"
                  "MODAL, STICKY, MAXIMIZED_VERT, MAXIMIZED_HORZ, SHADED, SKIP_TASKBAR, \n"
                  "SKIP_PAGER, HIDDEN, FULLSCREEN, ABOVE, BELOW, DEMANDS_ATTENTION\n";
  static int *errno_ptr;
  errno_ptr = &errno;
  while ((c = getopt_long_only(context->argc, context->argv, "+ha:r:t:",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'a':
        action = _NET_WM_STATE_ADD;
        arg_property = optarg;
        break;
      case 'r':
        action = _NET_WM_STATE_REMOVE;
        arg_property = optarg;
        break;
      case 't':
        action = _NET_WM_STATE_TOGGLE;
        arg_property = optarg;
        break;
      case 'h':
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (action == -1 || arg_property == NULL) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (!window_get_arg(context, 0, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  property = parse_property(arg_property);
  if (!property) {
    return 1;
  }

  window_each(context, window_arg, {
    ret = xdo_window_state(context->xdo, window, action, property);
    if (ret) {
      has_error = 1;
      fprintf(stderr, "xdo_window_property reported an error on window %ld\n",
              window);
    }
  }); /* window_each(...) */

  free(property);
  return has_error;
} /* int cmd_windowstate(context_t *) */
