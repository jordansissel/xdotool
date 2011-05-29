#include "xdo_cmd.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int cmd_sleep(context_t *context) {
  char *cmd = *context->argv;
  int ret = EXIT_SUCCESS;
  int c;
  double duration_usec;

  typedef enum {
    opt_unused, opt_help
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s seconds\n" \
    "Sleep a given number of seconds. Fractions of seconds are valid.\n";
  
  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "+h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc == 0) {
    fprintf(stderr, "No arguments given.\n");
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  duration_usec = atof(context->argv[0]) * (1000000);
  usleep(duration_usec);
  consume_args(context, 1);
  return ret;
}
