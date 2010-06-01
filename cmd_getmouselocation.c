#include "xdo_cmd.h"

int cmd_getmouselocation(int argc, char **args) {
  int x, y, screen_num;
  int ret;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "shell", no_argument, NULL, 's' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [--shell]\n"
    "--shell     - output shell variables for use with eval\n";
  int option_index;
  int output_shell = 0;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 's':
        output_shell = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 0) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ret = xdo_mouselocation(xdo, &x, &y, &screen_num);

  if (output_shell) {
    printf("X=%d\n", x);
    printf("Y=%d\n", y);
    printf("SCREEN=%d\n", screen_num);
  } else {
    printf("x:%d y:%d screen:%d\n", x, y, screen_num);
  }
  return ret;
}

