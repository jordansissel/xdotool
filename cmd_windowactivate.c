#include "xdo_cmd.h"

int cmd_windowactivate(int argc, char **args) {
  int ret = 0;
  Window wid;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s wid\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
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

  if (ret)
    fprintf(stderr, "xdo_window_activate reported an error\n");

  return ret;
}

