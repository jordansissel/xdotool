#include "xdo_cmd.h"

int cmd_mousedown(int argc, char **args) {
  int ret = 0;
  int button;
  char *cmd = *args;
  Window window = 0;
  xdo_active_mods_t *active_mods = NULL;
  int clear_modifiers = 0;

  int c;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
            "Usage: %s [--clearmodifiers] [--window WINDOW] <button>\n"
            "--window <windowid>    - specify a window to send keys to\n"
            "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "chw:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
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

  if (argc != 1) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  button = atoi(args[0]);

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  ret = xdo_mousedown(xdo, window, button);

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  if (ret) {
    fprintf(stderr, "xdo_mousedown reported an error\n");
  }

  return ret;
}
