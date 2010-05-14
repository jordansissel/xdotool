#include "xdo_cmd.h"

int cmd_type(int argc, char **args) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *args;
  xdo_active_mods_t *active_mods = NULL;

  /* Options */
  int clear_modifiers = 0;
  Window window = 0;
  useconds_t delay = 12000; /* 12ms between keystrokes default */

  struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "delay", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };

  static const char *usage =
            "Usage: %s [--window windowid] [--delay milliseconds] "
            "<things to type>\n"
            "--window <windowid>    - specify a window to send keys to\n"
            "--delay <milliseconds> - delay between keystrokes\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
            "-h, --help             - show this help output\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "w:d:ch", longopts, &option_index)) != -1) {
    switch (c) {
      case 'w':
        window = strtoul(optarg, NULL, 0);
        break;
      case 'd':
        /* --delay is in milliseconds, convert to microseconds */
        delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  args += optind;
  argc -= optind;

  if (argc == 0) {
    fprintf(stderr, "You specified the wrong number of args.\n");
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  for (i = 0; i < argc; i++) {
    int tmp = xdo_type(xdo, window, args[i], delay);

    if (tmp) {
      fprintf(stderr, "xdo_type reported an error\n");
    }

    ret += tmp;
  }

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret > 0;
}

