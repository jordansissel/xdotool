#define _GNU_SOURCE 1
#ifndef __USE_BSD
#define __USE_BSD /* for strdup on linux/glibc */
#endif /* __USE_BSD */
#include <string.h>


#include "xdo_cmd.h"

int cmd_set_window(context_t *context) {
  char *cmd = *context->argv;
  int c;
  char *role = NULL, *icon = NULL, *name = NULL, *_class = NULL,
       *classname = NULL;
  int override_redirect = -1;
  int urgency = -1;
  const char *window_arg = "%1";

  struct option longopts[] = {
    { "name", required_argument, NULL, 'n' },
    { "icon-name", required_argument, NULL, 'i' },
    { "role", required_argument, NULL, 'r' },
    { "class", required_argument, NULL, 'C' },
    { "classname", required_argument, NULL, 'N' },
    { "overrideredirect", required_argument, NULL, 'O' },
    { "urgency", required_argument, NULL, 'u' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  int option_index;
  static const char *usage = 
      "Usage: %s [options] [window=%1]\n"
      "--name NAME  - set the window name (aka title)\n"
      "--icon-name NAME - set the window name while minimized/iconified\n"
      "--role ROLE - set the window's role string\n"
      "--class CLASS - set the window's class\n"
      "--classname CLASSNAME - set the window's classname\n"
      "--overrideredirect OVERRIDE - set override_redirect.\n"
      "  1 means the window manager will not manage this window.\n"
      "--urgency URGENT - set the window's urgency hint.\n"
      "  1 sets the urgency flag, 0 removes it.\n";

  while ((c = getopt_long_only(context->argc, context->argv, "+hn:i:r:C:N:u:",
                               longopts, &option_index)) != -1) {
    switch(c) {
      case 'n': 
        name = strdup(optarg); 
        break;
      case 'i':
        icon = strdup(optarg);
        break;
      case 'r':
        role = strdup(optarg);
        break;
      case 'C':
        _class = strdup(optarg);
        break;
      case 'N':
        classname = strdup(optarg);
        break;
      case 'O':
        override_redirect = (atoi(optarg) > 0);
        break;
      case 'u':
        urgency = (atoi(optarg) > 0);
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

  /* adjust context->argc, argv */
  consume_args(context, optind);

  if (!window_get_arg(context, 0, 0, &window_arg)) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  /* TODO(sissel): error handling needed... */
  window_each(context, window_arg, {
    if (name)
      xdo_set_window_property(context->xdo, window, "WM_NAME", name);
    if (icon)
      xdo_set_window_property(context->xdo, window, "WM_ICON_NAME", icon);
    if (role)
      xdo_set_window_property(context->xdo, window, "WM_WINDOW_ROLE", role);
    if (classname || _class)
      xdo_set_window_class(context->xdo, window, classname, _class);
    if (override_redirect != -1)
      xdo_set_window_override_redirect(context->xdo, window,
                                       override_redirect);
    if (urgency != -1)
      xdo_set_window_urgency(context->xdo, window, urgency);
  }); /* window_each(...) */

  return 0;
}

