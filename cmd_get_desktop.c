#include "xdo_cmd.h"

int cmd_get_desktop(int argc, char **args) {
  int ret = 0;
  char *cmd = *args;
  long desktop = 0;

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

  ret = xdo_get_current_desktop(xdo, &desktop);
  printf("%ld\n", desktop);
  return ret;
}
