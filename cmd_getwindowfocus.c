#include "xdo_cmd.h"

/* Added 2007-07-28 - Lee Pumphret */
int cmd_getwindowfocus(int argc, char **args) {
  int ret = 0;
  int get_toplevel_focus = 1;
  Window wid = 0;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 'f' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s -f\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "fh", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 'f':
        get_toplevel_focus = 0;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc > 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (get_toplevel_focus) {
    ret = xdo_window_sane_get_focus(xdo, &wid);
  } else {
    ret = xdo_window_get_focus(xdo, &wid);
  }

  if (ret) {
    fprintf(stderr, "xdo_window_focus reported an error\n");
  } else { 
    window_print(wid);
  }

  return ret;
}

