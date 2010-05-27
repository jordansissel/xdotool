#include "xdo_cmd.h"
#include <math.h>

int cmd_mousemove(int argc, char **args) {
  int ret = 0;
  int x, y;
  char *cmd = *args;
  int opsync = 0;

  xdo_active_mods_t *active_mods = NULL;
  int clear_modifiers = 0;
  int polar_coordinates = 0;
  int step = 0;
  useconds_t delay = 0;

  int c;
  int screen = 0;
  Window window = 0;
  typedef enum {
    opt_unused, opt_help, opt_sync, opt_clearmodifiers, opt_polar,
    opt_screen, opt_step, opt_delay, opt_window
  } optlist_t;
  static struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, opt_clearmodifiers },
    { "help", no_argument, NULL, opt_help},
    { "polar", no_argument, NULL, opt_polar },
    { "screen", required_argument, NULL, opt_screen },
    //{ "step", required_argument, NULL, opt_step },
    { "sync", no_argument, NULL, opt_sync },
    //{ "delay", required_argument, NULL, opt_delay },
    { "window", required_argument, NULL, opt_window },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
             "Usage: %s [options] <x> <y>\n"
            "-c, --clearmodifiers      - reset active modifiers (alt, etc) while typing\n"
            "-d, --delay <MS>          - sleeptime in milliseconds between steps.\n"
            "-p, --polar               - Use polar coordinates. X as an angle, Y as distance\n"
            "--screen SCREEN           - which screen to move on, default is current screen\n"
            //"--step <STEP>             - pixels to move each time along path to x,y.\n"
            //"--sync                    - only exit once the window has been mapped (is visible)\n"
            "-w, --window <windowid>   - specify a window to move relative to\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "chw:pd:", longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
      case opt_clearmodifiers:
        clear_modifiers = 1;
        break;
      case 'h':
      case opt_help:
        printf(usage, cmd);
        return EXIT_SUCCESS;
        break;
      case opt_screen:
        screen = atoi(optarg);
        break;
      case 'w':
      case opt_window:
        window = strtoul(optarg, NULL, 0);
        break;
      case 'p':
      case opt_polar:
        polar_coordinates = 1;
        break;
      case opt_step:
        step = atoi(optarg);
        break;
      case 'd':
      case opt_delay:
        delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case opt_sync:
        opsync = 1;
        break;
      default:
        printf("unknown opt: %d\n", c);
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
      int win_x, win_y;
      unsigned int win_w, win_h;
      xdo_get_window_location(xdo, window, &win_x, &win_y, NULL);
      xdo_get_window_size(xdo, window, &win_w, &win_h);
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

    /* Negative sin, since screen Y coordinates are descending, where cartesian is ascending */
    y = origin_y + (-sin(radians) * distance);
  }

  int mx, my, mscreen;
  xdo_mouselocation(xdo, &mx, &my, &mscreen);

  /* Break early if we don't need to move */
  if (mx == x && my == y && mscreen == screen) {
    return 0;
  }
  //printf("Move from %d,%d => %d,%d\n", mx, my, x, y);

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

  if (ret) {
    fprintf(stderr, "xdo_mousemove reported an error\n");
  } else {
    if (opsync) {
      /* Wait until the mouse moves away from its current position */
      xdo_mouse_wait_for_move_from(xdo, mx, my);
    }
  }

  if (clear_modifiers) {
    xdo_set_active_modifiers(xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return ret;
}
