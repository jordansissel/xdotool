#include "xdo_cmd.h"
#include <stdio.h>
#include <time.h>

#ifdef __APPLE__
#  include "patch_clock_gettime.h"
#endif

int cmd_print_time(context_t *context) {
  char *cmd = *context->argv;

  /* Options processing largely just boilerplate right now, but someone may
   * want to add --format or fd=1,2,open(path) or who knows what later. */
  int c;
  enum { opt_unused, opt_help };
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { 0, 0, 0, 0 },
  };
  static const char *usage =
    "Usage: %s\n"
    "Print the current time as seconds.nanoseconds since the Unix epoch,\n"
    "using CLOCK_REALTIME. Intended for in action chains to timestamp events.\n"
    "Example: xdotool print_time key --window WID Return\n";
  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }
  consume_args(context, optind);

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  printf("%ld.%09ld\n", (long)ts.tv_sec, (long)ts.tv_nsec);
  fflush(stdout);
  return EXIT_SUCCESS;
}
