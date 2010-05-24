#include "xdo_cmd.h"
#include <math.h>

int cmd_mousemove(int argc, char **args) {
  int ret = 0;
  int x, y;
  char *cmd = *args;
  xdo_active_mods_t *active_mods = NULL;
  int clear_modifiers = 0;
  int polar_coordinates = 0;
  int step = 0;
  useconds_t delay = 0;

  int c;
  int screen = 0;
  Window window = 0;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "polar", no_argument, NULL, 'p' },
    { "screen", required_argument, NULL, 's' },
    { "step", required_argument, NULL, 't' },
    { "delay", required_argument, NULL, 'd' },
    { "window", required_argument, NULL, 'w' },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
             "Usage: %s [options] <x> <y>\n"
            "-c, --clearmodifiers      - reset active modifiers (alt, etc) while typing\n"
            "-s, -screen SCREEN        - which screen to move on, default is current screen\n"
            "-w, --window <windowid>   - specify a window to move relative to\n"
            "-p, --polar               - Use polar coordinates. X as an angle, Y as distance\n"
            "-t, --step <STEP>         - pixels to move each time along path to x,y.\n"
            "-d, --delay <MS>          - sleeptime in milliseconds between steps.\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "chs:w:pt:d:", longopts, &option_index)) != -1) {
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
      case 'p':
        polar_coordinates = 1;
        break;
      case 't':
        step = atoi(optarg);
        break;
      case 'd':
        delay = strtoul(optarg, NULL, 0) * 1000;
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

  if (polar_coordinates) {
    /* x becomes angle (degrees), y becomes distance.
     * XXX: Origin should be center (of window or screen)
     */
    int origin_x, origin_y;
    if (window > 0) {
      Window dummy_win;
      int win_x, win_y;
      unsigned int win_w, win_h, dummy_uint;
      XGetGeometry(xdo->xdpy, window, &dummy_win, &win_x, &win_y, &win_w, &win_h,
                   &dummy_uint, &dummy_uint);
      origin_x = win_x + (win_w / 2);
      origin_y = win_y + (win_h / 2);
    } else { /* no window selected, move relative to screen */
      Screen *s = ScreenOfDisplay(xdo->xdpy, screen);
      origin_x = s->width / 2;
      origin_y = s->height / 2;
    }
    double radians = (x * M_PI / 180);
    double distance = y;
    x = origin_x + (cos(radians) * distance);

    /* Negative sin, since screen Y coordinates are top-down, where cartesian is reverse */
    y = origin_y + (-sin(radians) * distance);
  }

  if (clear_modifiers) {
    active_mods = xdo_get_active_modifiers(xdo);
    xdo_clear_active_modifiers(xdo, window, active_mods);
  }

  if (step == 0) {
    if (window > 0) {
      ret = xdo_mousemove_relative_to_window(xdo, window, x, y);
    } else {
      ret = xdo_mousemove(xdo, x, y, screen);
    }
  } else {
    int mx, my, mscreen;
    //double xleg, yleg;
    xdo_mouselocation(xdo, &mx, &my, &mscreen);

    if (mx == x && my == y && mscreen == screen) {
      /* Nothing to move. Quit now. */
      return 0;
    }
    
    fprintf(stderr, "--step support not yet implemented\n");

    if (window > 0) {
      ret = xdo_mousemove_relative_to_window(xdo, window, x, y);
    } else {
      ret = xdo_mousemove(xdo, x, y, screen);
    }
  }

  if (ret)
    fprintf(stderr, "xdo_mousemove reported an error\n");

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret;
}
