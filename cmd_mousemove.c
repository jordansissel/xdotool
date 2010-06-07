#include "xdo_cmd.h"
#include <math.h>
#include <string.h>

struct mousemove {
  Window window;
  int clear_modifiers;
  int opsync;
  int polar_coordinates;
  int x;
  int y;
  int screen;
  useconds_t delay;

  int step;
};

static int _mousemove(context_t *context, struct mousemove *mousemove);

int cmd_mousemove(context_t *context) {
  int ret = 0;
  char *cmd = *context->argv;
  char *window_arg = NULL;

  struct mousemove mousemove;
  mousemove.clear_modifiers = 0;
  mousemove.polar_coordinates = 0;
  mousemove.opsync = 0;
  mousemove.screen = 0;
  mousemove.x = 0;
  mousemove.y = 0;
  mousemove.step = 0;

  int c;
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
      //"-d, --delay <MS>          - sleeptime in milliseconds between steps.\n"
      //"--step <STEP>             - pixels to move each time along path to x,y.\n" "-p, --polar               - Use polar coordinates. X as an angle, Y as distance\n"
      "--screen SCREEN           - which screen to move on, default is current screen\n"
      "--sync                    - only exit once the mouse has moved\n"
      "-w, --window <windowid>   - specify a window to move relative to.\n";
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "chw:pd:",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'c':
      case opt_clearmodifiers:
        mousemove.clear_modifiers = 1;
        break;
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case opt_screen:
        mousemove.screen = atoi(optarg);
        break;
      case 'w':
      case opt_window:
        window_arg = strdup(optarg);
        break;
      case 'p':
      case opt_polar:
        mousemove.polar_coordinates = 1;
        break;
      case opt_step:
        mousemove.step = atoi(optarg);
        break;
      case 'd':
      case opt_delay:
        mousemove.delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case opt_sync:
        mousemove.opsync = 1;
        break;
      default:
        printf("unknown opt: %d\n", c);
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc < 2) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args (expected 2).\n");
    return 1;
  }

  mousemove.x = atoi(context->argv[0]);
  mousemove.y = atoi(context->argv[1]);

  consume_args(context, 2);

  window_each(context, window_arg, {
    mousemove.window = window;
    ret = _mousemove(context, &mousemove);
    if (ret != XDO_SUCCESS) {
      return ret;
    }
  }); /* window_each(...) */

  return ret;
}

static int _mousemove(context_t *context, struct mousemove *mousemove) {
  int ret;
  xdo_active_mods_t *active_mods = NULL;

  int x = mousemove->x;
  int y = mousemove->y;
  int screen = mousemove->screen;
  Window window = mousemove->window;
  
  if (mousemove->polar_coordinates) {
    /* x becomes angle (degrees), y becomes distance.
     * XXX: Origin should be center (of window or screen)
     */
    int origin_x, origin_y;
    if (mousemove->window != CURRENTWINDOW) {
      int win_x, win_y;
      unsigned int win_w, win_h;
      xdo_get_window_location(context->xdo, window, &win_x, &win_y, NULL);
      xdo_get_window_size(context->xdo, window, &win_w, &win_h);
      origin_x = win_x + (win_w / 2);
      origin_y = win_y + (win_h / 2);
    } else { /* no window selected, move relative to screen */
      Screen *s = ScreenOfDisplay(context->xdo->xdpy, screen);
      origin_x = s->width / 2;
      origin_y = s->height / 2;
    }

    /* The original request for polar support was that '0' degrees is up
     * and that rotation was clockwise, so 0 is up, 90 right, 180 down, 270
     * left. This conversion can be done with (360 - degrees) + 90 */
    //double radians = (x * M_PI / 180);
    double radians = ((360 - x) + 90) * M_PI / 180;
    double distance = y;
    x = origin_x + (cos(radians) * distance);

    /* Negative sin, since screen Y coordinates are descending, where cartesian
     * is ascending */
    y = origin_y + (-sin(radians) * distance);
  }

  int mx, my, mscreen;
  xdo_mouselocation(context->xdo, &mx, &my, &mscreen);

  /* Break early if we don't need to move */
  if (mx == x && my == y && mscreen == screen) {
    return 0;
  }

  if (mousemove->clear_modifiers) {
    active_mods = xdo_get_active_modifiers(context->xdo);
    xdo_clear_active_modifiers(context->xdo, window, active_mods);
  }

  if (mousemove->step == 0) {
    if (window != CURRENTWINDOW && !mousemove->polar_coordinates) {
      ret = xdo_mousemove_relative_to_window(context->xdo, window, x, y);
    } else {
      ret = xdo_mousemove(context->xdo, x, y, screen);
    }
  } else {
    if (mx == x && my == y && mscreen == screen) {
      /* Nothing to move. Quit now. */
      return 0;
    }
    
    fprintf(stderr, "--step support not yet implemented\n");

    if (window > 0) {
      ret = xdo_mousemove_relative_to_window(context->xdo, window, x, y);
    } else {
      ret = xdo_mousemove(context->xdo, x, y, screen);
    }
  }

  if (ret) {
    fprintf(stderr, "xdo_mousemove reported an error\n");
  } else {
    if (mousemove->opsync) {
      /* Wait until the mouse moves away from its current position */
      xdo_mouse_wait_for_move_from(context->xdo, mx, my);
    }
  }

  if (mousemove->clear_modifiers) {
    xdo_set_active_modifiers(context->xdo, window, active_mods);
    xdo_free_active_modifiers(active_mods);
  }

  return 0;
} /* int mousemove ... */
