#include "xdo_cmd.h"

int cmd_mousemove(int argc, char **args) {
  int ret = 0;
  int x, y;
  char *cmd = *args;
  xdo_active_mods_t *active_mods = NULL;
  int clear_modifiers = 0;

  int c;
  int screen = 0;
  Window window = 0;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "screen", required_argument, NULL, 's' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
             "Usage: %s [options] <xcoord> <ycoord>\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
            "--screen SCREEN        - which screen to move on, default is current screen\n"
            "--window <windowid>    - specify a window to move relative to\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "chs:w:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 's':
        screen = atoi(optarg);
        break;
      case 'w':
        window = strtoul(optarg, NULL, 0);
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 2) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  x = atoi(args[0]);
  y = atoi(args[1]);

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  if (window > 0) {
    ret = xdo_mousemove_relative_to_window(xdo, window, x, y);
  } else {
    ret = xdo_mousemove(xdo, x, y, screen);
  }

  if (ret)
    fprintf(stderr, "xdo_mousemove reported an error\n");

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret;
}
