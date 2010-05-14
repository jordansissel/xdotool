#include "xdo_cmd.h"

int cmd_getwindowpid(int argc, char **args) {
  Window wid = 0;
  int pid;
  char *cmd = *args;

  int c;

  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = "Usage: %s <window id>\n";
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
  pid = xdo_window_get_pid(xdo, wid);
  if (pid == 0) {
    fprintf(stderr, "window %ld has no pid associated with it.\n", wid);
  }

  printf("%d\n", pid);
  return pid != 0;
}

