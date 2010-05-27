#include "xdo_cmd.h"
#include <math.h>

int cmd_mousemove_relative(int argc, char **args) {
  int x, y;
  int ret = 0;
  char *cmd = *args;
  int polar_coordinates;

  int c;
  static struct option longopts[] = {
    { "help", no_argument, NULL, 'h' },
    { "polar", no_argument, NULL, 'p' },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
            "Usage: %s [--polar] <x> <y>\n"
            "Using polar coordinate mode makes 'x' the angle (in degrees) and\n"
            "'y' the distance.\n"
            "\n"
            "If you want to use negative numbers for a coordinate,\n"
            "you'll need to invoke it this way (with the '--'):\n"
            "  %s -- -20 -15\n"
            "otherwise, normal usage looks like this:\n"
            "  %s 100 140\n";
  int option_index;

  while ((c = getopt_long_only(argc, args, "h", longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
        printf(usage, cmd, cmd, cmd);
        return EXIT_SUCCESS;
        break;
      case 'p':
        polar_coordinates = 1;
        break;
      default:
        fprintf(stderr, usage, cmd, cmd, cmd);
        return EXIT_FAILURE;
    }
  }

  argc -= optind;
  args += optind;

  if (argc != 2) {
    fprintf(stderr, usage, cmd, cmd, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return 1;
  }

  x = atoi(args[0]);
  y = atoi(args[1]);

  if (polar_coordinates) {
    /* x becomes angle (degrees), y becomes distance.
     * XXX: Origin should be center (of window or screen)
     */
    int origin_x, origin_y, screen;
    xdo_mouselocation(xdo, &origin_x, &origin_y, &screen);

    double radians = (x * M_PI / 180);
    double distance = y;
    x = origin_x + (cos(radians) * distance);
    /* Negative sin, since screen Y coordinates are top-down, where cartesian is reverse */
    y = origin_y + (-sin(radians) * distance);
    //printf("%d,%d => %d,%d\n", origin_x, origin_y, x, y);
    ret = xdo_mousemove(xdo, x, y, screen);
  } else {
    ret = xdo_mousemove_relative(xdo, x, y);
  }

  if (ret)
    fprintf(stderr, "xdo_mousemove_relative reported an error\n");

  return ret;
}

