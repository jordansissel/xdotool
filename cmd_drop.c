#include "xdo_cmd.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include <X11/Xatom.h>

int cmd_drop(context_t *context) {
  char *cmd = *context->argv;
  char **command = NULL;
  int command_count = 0;
  int ret = EXIT_SUCCESS;
  int arity = -1;
  char *terminator = NULL;
  int c, i;

  enum { opt_unused, opt_help, opt_args, opt_terminator };
  static struct option longopts[] = {
      {"help", no_argument, NULL, opt_help},
      {"args", required_argument, NULL, opt_args},
      {"terminator", required_argument, NULL, opt_terminator},
      {0, 0, 0, 0},
  };
  static const char *usage =
      "Usage: %s [options] mimetype value [mimetype1 value1 [mimetype2 "
      "value2]] [terminator]\n"
      "--args N  - how many mimetype and value pairs to expect in the drop\n"
      "            command. This is useful for ending a drop and continuing\n"
      "            with more xdotool commands\n"
      "--terminator TERM - similar to --args, specifies a terminator that\n"
      "                    marks the end of 'drop' arguments. This is useful\n"
      "                    for continuing with more xdotool commands.\n"
      "\n"
      "Unless --args OR --terminator is specified, the drop command is "
      "assumed\n"
      "to be the remainder of the command line.\n";

  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "+h", longopts,
                               &option_index)) != -1) {
    switch (c) {
    case 'h':
    case opt_help:
      printf(usage, cmd);
      consume_args(context, context->argc);
      return EXIT_SUCCESS;
      break;
    case opt_args:
      arity = atoi(optarg);
      break;
    case opt_terminator:
      terminator = strdup(optarg);
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

  if (arity > 0 && terminator != NULL) {
    fprintf(stderr, "Don't use both --terminator and --args.\n");
    return EXIT_FAILURE;
  }

  if (context->argc < arity) {
    fprintf(stderr, "You said '--args %d' but only gave %d arguments.\n", arity,
            context->argc);
    return EXIT_FAILURE;
  }

  command = calloc(context->argc + 1, sizeof(char *));

  for (i = 0; i < context->argc; i++) {
    if (arity > 0 && i == arity) {
      break;
    }

    /* if we have a terminator and the current argument matches it... */
    if (terminator != NULL && strcmp(terminator, context->argv[i]) == 0) {
      command_count++; /* Consume the terminator, too */
      break;
    }

    command[i] = strdup(context->argv[i]);
    command_count = i + 1; /* i starts at 0 */
    xdotool_debug(context, "drop arg[%d]: %s", i, command[i]);
  }

  if (command_count % 2 != 0) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return EXIT_FAILURE;
  }

  if (command_count / 2 > 3) {
    fprintf(stderr, usage, cmd);
    fprintf(stderr, "Only 3 mimetype/value pairs are supported.\n");
  }

  command[i] = NULL;

  // Prepare raw format array and values array
  char **formats = calloc(command_count/2, sizeof(char *));
  char **values = calloc(command_count/2, sizeof(char *));
  for (int i=0; i<command_count; i+=2) {
    formats[i/2] = command[i];
    values[i/2] = command[i+1];
  }

  // Find window under cursor
  int x;
  int y;
  Window target;
  ret = xdo_get_mouse_location2(context->xdo, &x, &y, NULL, &target);
  xdotool_debug(context, "Target window (at coordinates %d,%d) has for id %d",
                x, y, target);

  ret = xdo_drop(context->xdo, target, x, y, formats, values, command_count/2) == XDO_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;

  free(formats);
  free(values);
  consume_args(context, command_count);
  free(terminator);

  for (i = 0; i < command_count; i++) {
    free(command[i]);
  }
  free(command);
  return ret;
}
