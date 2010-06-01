#include "xdo_cmd.h"
#include <string.h> /* for strcmp */

/* This function handles all of these commands:
 * xdotool key
 * xdtoool keyup
 * xdotool keydown
 */

int cmd_key(int argc, char **args) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *args;
  xdo_active_mods_t *active_mods = NULL;
  useconds_t delay = 12000;

  /* Options */
  Window window = 0;
  int clear_modifiers = 0;

  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "delay", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };

  static const char *usage = 
     "Usage: %s [options] <keysequence> [keysequence ...]\n"
     "--clearmodifiers     - clear active keyboard modifiers during keystrokes\n"
     "--delay DELAY        - Use DELAY milliseconds between keystrokes\n"
     "--window WINDOW      - send keystrokes to a specific window\n"
     "Each keysequence can be any number of modifiers and keys, separated by plus (+)\n"
     "For example: alt+r\n"
     "Any letter or key symbol such as Shift_L, Return, Dollar, a, space are valid.\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "d:hcw:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'w':
        window = strtoul(optarg, NULL, 0);
        break;
      case 'c':
        clear_modifiers = 1;
        break;
      case 'h':
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case 'd':
        /* Argument is in milliseconds, keysequence delay is in microseconds. */
        delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc == 0) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  int (*keyfunc)(const xdo_t *, Window, const char *, useconds_t) = NULL;

  if (!strcmp(cmd, "key")) {
    keyfunc = xdo_keysequence;
  } else if (!strcmp(cmd, "keyup")) {
    keyfunc = xdo_keysequence_up;
  } else if (!strcmp(cmd, "keydown")) {
    keyfunc = xdo_keysequence_down;
  } else {
    fprintf(stderr, "Unknown command '%s'\n", cmd);
    return 1;
  }

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  for (i = 0; i < argc; i++) {
    int tmp = keyfunc(xdo, window, args[i], delay);
    if (tmp != 0)
      fprintf(stderr, "xdo_keysequence reported an error for string '%s'\n", args[i]);
    ret += tmp;
  }

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret;
}

