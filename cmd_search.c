#include "xdo_cmd.h"
#include <string.h>

int cmd_search(context_t *context) {
  Window *list;
  xdo_search_t search;
  int nwindows;
  int i;
  int c;

  int search_title = 0;
  int search_name = 0;
  int search_class = 0;
  int search_classname = 0;
  typedef enum { 
    opt_unused, opt_title, opt_onlyvisible, opt_name, opt_class, opt_maxdepth,
    opt_pid, opt_help, opt_any, opt_all, opt_screen, opt_classname
  } optlist_t;
  struct option longopts[] = {
    { "all", no_argument, NULL, opt_all },
    { "any", no_argument, NULL, opt_any },
    { "class", no_argument, NULL, opt_class },
    { "classname", no_argument, NULL, opt_classname },
    { "help", no_argument, NULL, opt_help },
    { "maxdepth", required_argument, NULL, opt_maxdepth },
    { "name", no_argument, NULL, opt_name },
    { "onlyvisible", 0, NULL, opt_onlyvisible },
    { "pid", required_argument, NULL, opt_pid },
    { "screen", required_argument, NULL, opt_screen },
    { "title", no_argument, NULL, opt_title },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
      "Usage: xdotool %s "
      "[options] regexp_pattern\n"
      "--class         check regexp_pattern agains the window class\n"
      "--classname     check regexp_pattern agains the window classname\n"
      "--maxdepth N    set search depth to N. Default is infinite.\n"
      "                -1 also means infinite.\n"
      "--onlyvisible   matches only windows currently visible\n"
      "--pid PID       only show windows belonging to specific process\n"
      "                Not supported by all X11 applications\n"
      "--screen N      only search a specific screen. Default is all screens\n"
      "--name          check regexp_pattern agains the window name\n"
      "--title         DEPRECATED. Same as --name.\n"
      "--all           Require all conditions match a window. Default is --any\n"
      "--any           Windows matching any condition will be reported\n"
      "-h, --help      show this help output\n"
      "\n"
      "If none of --name, --classname, or --class are specified, the \n"
      "defaults are: --name --classname --class\n";

  memset(&search, 0, sizeof(xdo_search_t));
  search.max_depth = -1;
  search.require = SEARCH_ANY;

  char *cmd = *context->argv;
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 0:
        break;
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
      case opt_maxdepth:
        search.max_depth = strtol(optarg, NULL, 0);
        break;
      case opt_pid:
        search.pid = atoi(optarg);
        search.searchmask |= SEARCH_PID;
        break;
      case opt_any:
        search.require = SEARCH_ANY;
        break;
      case opt_all:
        search.require = SEARCH_ALL;
        break;
      case opt_screen:
        search.screen = strtoul(optarg, NULL, 0);
        search.searchmask |= SEARCH_SCREEN;
        break;
      case opt_onlyvisible:
        search.only_visible = True;
        search.searchmask |= SEARCH_ONLYVISIBLE;
        break;
      case opt_class:
        search_class = True;
        break;
      case opt_classname:
        search_classname = True;
        break;
      case opt_title:
        fprintf(stderr, "This flag is deprecated. Assuming you mean --name (the"
                " window name).\n");
        /* fall through */
      case opt_name:
        search_name = True;
        break;
      default:
        fprintf(stderr, "Invalid usage\n");
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  /* We require a pattern or a pid to search for */
  if (context->argc < 1 && search.pid == 0) {
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  if (!search_title && !search_name && !search_class && !search_classname 
      && context->argc > 0) {
    fprintf(stderr, "Defaulting to search window name, class, and classname\n");
    search.searchmask |= (SEARCH_NAME | SEARCH_CLASS | SEARCH_CLASSNAME);
    search_name = 1;
    search_class = 1;
    search_classname = 1;
  }

  if (context->argc > 0) {
    if (search_title) {
      search.searchmask |= SEARCH_NAME;
      search.winname = context->argv[0];
    }
    if (search_name) {
      search.searchmask |= SEARCH_NAME;
      search.winname = context->argv[0];
    }
    if (search_class) {
      search.searchmask |= SEARCH_CLASS;
      search.winclass = context->argv[0];
    }
    if (search_classname) {
      search.searchmask |= SEARCH_CLASSNAME;
      search.winclassname = context->argv[0];
    }
  }

  xdo_window_search(context->xdo, &search, &list, &nwindows);
  for (i = 0; i < nwindows; i++) {
    window_print(list[i]);
  }

  /* Free list as it's malloc'd by xdo_window_search */
  free(list);

  consume_args(context, 1);

  /* error if number of windows found is zero (behave like grep) */
  return (nwindows ? EXIT_SUCCESS : EXIT_FAILURE);
}
