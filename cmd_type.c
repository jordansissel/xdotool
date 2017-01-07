#include "xdo_cmd.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

int cmd_type(context_t *context) {
  int ret = 0;
  int i;
  int c;
  char *cmd = *context->argv;
  const char *window_arg = NULL;
  int arity = -1;
  char *terminator = NULL;
  char *file = NULL;

  FILE *input = NULL;
  char *buffer = NULL;
  char *marker = NULL;
  size_t bytes_read = 0;

  char **data = NULL; /* stuff to type */
  int data_count = 0;
  int args_count = 0;
  charcodemap_t *active_mods = NULL;
  int active_mods_n;

  /* Options */
  int clear_modifiers = 0;
  useconds_t delay = 12000; /* 12ms between keystrokes default */

  typedef enum {
    opt_unused, opt_clearmodifiers, opt_delay, opt_help, opt_window, opt_args,
    opt_terminator, opt_file
  } optlist_t;

  struct option longopts[] = {
    { "clearmodifiers", no_argument, NULL, opt_clearmodifiers },
    { "delay", required_argument, NULL, opt_delay },
    { "help", no_argument, NULL, opt_help },
    { "window", required_argument, NULL, opt_window },
    { "args", required_argument, NULL, opt_args },
    { "terminator", required_argument, NULL, opt_terminator },
    { "file", required_argument, NULL, opt_file },
    { 0, 0, 0, 0 },
  };

  static const char *usage =
    "Usage: %s [--window windowid] [--delay milliseconds] "
    "<things to type>\n"
    "--window <windowid>    - specify a window to send keys to\n"
    "--delay <milliseconds> - delay between keystrokes\n"
    "--clearmodifiers       - reset active modifiers (alt, etc) while typing\n"
    "--args N  - how many arguments to expect in the exec command. This is\n"
    "            useful for ending an exec and continuing with more xdotool\n"
    "            commands\n"
    "--terminator TERM - similar to --args, specifies a terminator that\n"
    "                    marks the end of 'exec' arguments. This is useful\n"
    "                    for continuing with more xdotool commands.\n"
    "--file <filepath> - specify a file, the contents of which will be\n"
    "                    be typed as if passed as an argument. The filepath\n"
    "                    may also be '-' to read from stdin.\n"
            "-h, --help             - show this help output\n"
    HELP_SEE_WINDOW_STACK;
  int option_index;

  while ((c = getopt_long_only(context->argc, context->argv, "+w:d:ch",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case opt_window:
        window_arg = optarg;
        break;
      case opt_delay:
        /* --delay is in milliseconds, convert to microseconds */
        delay = strtoul(optarg, NULL, 0) * 1000;
        break;
      case opt_clearmodifiers:
        clear_modifiers = 1;
        break;
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
      case opt_file:
	file = strdup(optarg);
	break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc == 0 && file == NULL) {
    fprintf(stderr, "You specified the wrong number of args.\n");
    fprintf(stderr, usage, cmd);
    return 1;
  }

  if (arity > 0 && terminator != NULL) {
    fprintf(stderr, "Don't use both --terminator and --args.\n");
    return EXIT_FAILURE;
  }

  if (context->argc < arity) {
    fprintf(stderr, "You said '--args %d' but only gave %d arguments.\n",
            arity, context->argc);
    return EXIT_FAILURE;
  }

  if (file != NULL) {
    data = calloc(1 + context->argc, sizeof(char *));

    /* determine whether reading from a file or from stdin */
    if (!strcmp(file, "-")) {
      input = fdopen(0, "r");
    } else {
      input = fopen(file, "r");
      if (input == NULL) {
        fprintf(stderr, "Failure opening '%s': %s\n", file, strerror(errno));
        return EXIT_FAILURE;
      }
    }

    while (feof(input) == 0) {
      marker = realloc(buffer, bytes_read + 4096);
      if (marker == NULL) {
        fprintf(stderr, "Failure allocating for '%s': %s\n", file, strerror(errno));
        return EXIT_FAILURE;
      }

      buffer = marker;
      marker = buffer + bytes_read;
      if (fgets(marker, 4096, input) != NULL) {
        bytes_read = (marker - buffer) + strlen(marker);
      } else {
        *marker = 0;
      }

      if (ferror(input) != 0) {
        fprintf(stderr, "Failure reading '%s': %s\n", file, strerror(errno));
        return EXIT_FAILURE;
      }
    }

    data[0] = buffer;
    data_count++;

    fclose(input);
  }
  else {
    data = calloc(context->argc, sizeof(char *));
  }

  /* Apply any --arity or --terminator */
  for (i=0; i < context->argc; i++) {
    if (arity > 0 && i == arity) {
      data[data_count] = NULL;
      break;
    }

    /* if we have a terminator and the current argument matches it... */
    if (terminator != NULL && strcmp(terminator, context->argv[i]) == 0) {
      data[data_count] = NULL;
      args_count++; /* Consume the terminator, too */
      break;
    }

    data[data_count] = strdup(context->argv[i]);
    xdotool_debug(context, "Exec arg[%d]: %s", i, data[data_count]);
    data_count++;
    args_count++;
  }

  window_each(context, window_arg, {
    if (clear_modifiers) {
      xdo_get_active_modifiers(context->xdo, &active_mods, &active_mods_n);
      xdo_clear_active_modifiers(context->xdo, window, active_mods, active_mods_n);
    }

    for (i = 0; i < data_count; i++) {
      //printf("Typing: '%s'\n", context->argv[i]);
      int tmp = xdo_enter_text_window(context->xdo, window, data[i], delay);

      if (tmp) {
        fprintf(stderr, "xdo_enter_text_window reported an error\n");
      }

      ret += tmp;
    }

    if (clear_modifiers) {
      xdo_set_active_modifiers(context->xdo, window, active_mods, active_mods_n);
      free(active_mods);
    }
  }); /* window_each(...) */

  free(data);

  consume_args(context, args_count);
  return ret > 0;
}

