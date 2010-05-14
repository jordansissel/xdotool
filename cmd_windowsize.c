#include "xdo_cmd.h"

int cmd_windowsize(int argc, char **args) {
  int ret = 0;
  int width, height;
  Window wid;
  int c;

  int use_hints = 0;
  struct option longopts[] = {
    { "usehints", 0, NULL, 'u' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };

  int size_flags = 0;
  char *cmd = *args;
  int option_index;
  static const char *usage =
            "Usage: %s [--usehints] windowid width height\n" \
            "--usehints  - Use window sizing hints (like characters in terminals)\n";

  while ((c = getopt_long_only(argc, args, "uh", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
      case 'u':
        use_hints = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  if (use_hints)
    size_flags |= SIZE_USEHINTS;

  args += optind;
  argc -= optind;

  if (argc != 3) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  wid = (Window)strtol(args[0], NULL, 0);
  width = (int)strtol(args[1], NULL, 0);
  height = (int)strtol(args[2], NULL, 0);

  ret = xdo_window_setsize(xdo, wid, width, height, size_flags);
  if (ret)
    fprintf(stderr, "xdo_window_setsize reported an error\n");
  return ret;
}

