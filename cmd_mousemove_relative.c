#include "xdo_cmd.h"

int cmd_mousemove_relative(int argc, char **args) {
  int x, y;
  int ret = 0;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
            "Usage: %s <xcoord> <ycoord>\n"
            "If you want to use negative numbers for a coordinate,\n"
            "you'll need to invoke it this way (with the '--'):\n"
            "  %s -- -20 -15\n"
            "otherwise, normal usage looks like this:\n"
            "  %s 100 140\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd, cmd, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd, cmd, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 2) {
    fprintf(stderr, usage, cmd, cmd, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  x = atoi(args[0]);
  y = atoi(args[1]);

  ret = xdo_mousemove_relative(xdo, x, y);

  if (ret)
    fprintf(stderr, "xdo_mousemove_relative reported an error\n");

  return ret;
}

