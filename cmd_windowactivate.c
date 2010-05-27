#include "xdo_cmd.h"

int cmd_windowactivate(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args;
  int opsync = 0;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_sync
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] wid\n"
    "--sync    - only exit once the window is active (is visible + active)\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case opt_sync:
        opsync = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  ret = xdo_window_activate(xdo, wid);

  if (ret) {
    fprintf(stderr, "xdo_window_activate reported an error\n");
  } else {
    if (opsync) {
      xdo_window_wait_for_active(xdo, wid, 1);
    }
  }

  return ret;
}

