#include "xdo_cmd.h"

int cmd_getactivewindow(int argc, char **args) {
  Window wid = 0;
  int ret;
  char *cmd = *args;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s\n";
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

  if (argc != 0) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  ret = xdo_window_get_active(xdo, &wid);

  if (ret) {
    fprintf(stderr, "xdo_get_active_window reported an error\n");
  } else {
    window_print(wid);
  }

  return ret;
}

