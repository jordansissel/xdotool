#include "xdo_cmd.h"

int cmd_get_desktop_for_window(int argc, char **args) {
  int ret = 0;
  char *cmd = *args;
  long desktop = 0;
  Window window = 0;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window>\n";
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

  window = (Window)strtol(args[0], NULL, 0);

  ret = xdo_get_desktop_for_window(xdo, window, &desktop);
  printf("%ld\n", desktop);
  return ret;
}

