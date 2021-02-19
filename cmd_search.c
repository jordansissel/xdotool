#include "xdo_cmd.h"
#include <string.h>

int cmd_search(context_t *context) {
  Window *list = NULL;
  xdo_search_t search;
  unsigned int nwindows;
  unsigned int i;
  int c;
  int op_sync = False;

  int search_title = 0;
  int search_name = 0;
  int out_shell = 0;
  char out_prefix[17] = {'\0'};
  int search_class = 0;
  int search_classname = 0;
  int search_role = 0;
  typedef enum {
    opt_unused, opt_title, opt_onlyvisible, opt_name, opt_shell, opt_prefix, opt_class, opt_maxdepth,
    opt_pid, opt_help, opt_any, opt_all, opt_screen, opt_classname, opt_desktop,
    opt_limit, opt_sync
    ,opt_role
  } optlist_t;
  struct option longopts[] = {
    { "all", no_argument, NULL, opt_all },
    { "any", no_argument, NULL, opt_any },
    { "class", no_argument, NULL, opt_class },
    { "classname", no_argument, NULL, opt_classname },
    { "help", no_argument, NULL, opt_help },
    { "maxdepth", required_argument, NULL, opt_maxdepth },
    { "name", no_argument, NULL, opt_name },
    { "shell", no_argument, NULL, opt_shell },
    { "prefix", required_argument, NULL, opt_prefix },
    { "onlyvisible", 0, NULL, opt_onlyvisible },
    { "pid", required_argument, NULL, opt_pid },
    { "screen", required_argument, NULL, opt_screen },
    { "title", no_argument, NULL, opt_title },
    { "desktop", required_argument, NULL, opt_desktop },
    { "limit", required_argument, NULL, opt_limit },
    { "sync", no_argument, NULL, opt_sync },
    { "role", no_argument, NULL, opt_role },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
      "Usage: xdotool %s "
      "[options] regexp_pattern\n"
      "--class         check regexp_pattern against the window class\n"
      "--classname     check regexp_pattern against the window classname\n"
      "--role          check regexp_pattern against the window role\n"
      "--maxdepth N    set search depth to N. Default is infinite.\n"
      "                -1 also means infinite.\n"
      "--onlyvisible   matches only windows currently visible\n"
      "--pid PID       only show windows belonging to specific process\n"
      "                Not supported by all X11 applications\n"
      "--screen N      only search a specific screen. Default is all screens\n"
      "--desktop N     only search a specific desktop number\n"
      "--limit N       break search after N results\n"
      "--name          check regexp_pattern against the window name\n"
      "--shell         print results as shell array WINDOWS=( ... )\n"
      "--prefix STR    use prefix (max 16 chars) for array name STRWINDOWS\n"
      "--title         DEPRECATED. Same as --name.\n"
      "--all           Require all conditions match a window. Default is --any\n"
      "--any           Windows matching any condition will be reported\n"
      "--sync          Wait until a search result is found.\n"
      "-h, --help      show this help output\n"
      "\n"
      "If none of --name, --classname, --class, or --role are specified, the \n"
      "defaults are: --name --classname --class --role\n";

  memset(&search, 0, sizeof(xdo_search_t));
  search.max_depth = -1;
  search.require = SEARCH_ANY;

  char *cmd = *context->argv;
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+h",
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
      case opt_role:
        search_role = True;
        break;
      case opt_title:
        fprintf(stderr, "This flag is deprecated. Assuming you mean --name (the"
                " window name).\n");
        /* fall through */
      case opt_name:
        search_name = True;
        break;
      case opt_shell:
        out_shell = True;
        break;
      case opt_prefix:
        strncpy(out_prefix, optarg, sizeof(out_prefix)-1);
        out_prefix[ sizeof(out_prefix)-1 ] = '\0'; //just in case
        break;
      case opt_desktop:
        search.desktop = strtol(optarg, NULL, 0);
        search.searchmask |= SEARCH_DESKTOP;
        break;
      case opt_limit:
        search.limit = atoi(optarg);
        break;
      case opt_sync:
        op_sync = True;
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

  if (context->argc > 0) {
    if (!search_title && !search_name && !search_class && !search_classname
        && !search_role) {
      fprintf(stderr,
        "Defaulting to search window name, class, classname, and role\n");
      search.searchmask |= (SEARCH_NAME | SEARCH_CLASS | SEARCH_CLASSNAME
        | SEARCH_ROLE);
      search_name = 1;
      search_class = 1;
      search_classname = 1;
      search_role = 1;
    }

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
    if (search_role) {
      search.searchmask |= SEARCH_ROLE;
      search.winrole = context->argv[0];
    }
    consume_args(context, 1);
  }

  do {
    free(list);

    xdo_search_windows(context->xdo, &search, &list, &nwindows);

    if ( (context->argc == 0) || out_shell ) {
      /* only print if we're the last command or printing to shell*/
      if (out_shell) printf("%s%s", out_prefix, "WINDOWS=(");
      for (i = 0; i < nwindows; i++) {
        window_print(list[i]);
      }
      if (out_shell) printf("%s",")\n");
    }

    if (op_sync && nwindows == 0) {
      xdotool_debug(context, "No search results, still waiting...");

      /* TODO(sissel): Make this tunable */
      usleep(500000);
    }
  } while (op_sync && nwindows == 0);

  /* Free old list as it's malloc'd by xdo_search_windows */
  free(context->windows);
  context->windows = list;
  context->nwindows = nwindows;

  /* error if number of windows found is zero (behave like grep) 
  but return success when being used inside eval (--shell option)*/
  return (nwindows || out_shell ? EXIT_SUCCESS : EXIT_FAILURE);
}
