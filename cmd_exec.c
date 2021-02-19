#include "xdo_cmd.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int cmd_exec(context_t *context) {
  char *cmd = *context->argv;
  char **command = NULL;
  int command_count = 0;
  int ret = EXIT_SUCCESS;
  int opsync = 0;
  int arity = -1;
  char *terminator = NULL;
  int c, i;

  typedef enum {
    opt_unused, opt_help, opt_sync, opt_args, opt_terminator
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { "args", required_argument, NULL, opt_args },
    { "terminator", required_argument, NULL, opt_terminator },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] command [arg1 arg2 ...] [terminator]\n"
    "--sync    - only exit when the command given finishes. The default\n"
    "            is to fork a child process and continue.\n"
    "--args N  - how many arguments to expect in the exec command. This is\n"
    "            useful for ending an exec and continuing with more xdotool\n"
    "            commands\n"
    "--terminator TERM - similar to --args, specifies a terminator that\n"
    "                    marks the end of 'exec' arguments. This is useful\n"
    "                    for continuing with more xdotool commands.\n"
    "\n"
    "Unless --args OR --terminator is specified, the exec command is assumed\n"
    "to be the remainder of the command line.\n";
  
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
      case opt_sync:
        opsync = 1;
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
    fprintf(stderr, "You said '--args %d' but only gave %d arguments.\n",
            arity, context->argc);
    return EXIT_FAILURE;
  }

  command = calloc(context->argc + 1, sizeof(char *));

  for (i=0; i < context->argc; i++) {
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
    xdotool_debug(context, "Exec arg[%d]: %s", i, command[i]);
  }
  command[i] = NULL;
  
  pid_t child;
  child = fork();
  if (child == 0) { /* child */
    execvp(command[0], command);

    /* if we get here, there was an error */
    perror("execvp failed");
    exit(errno);
  } else { /* parent */
    if (opsync) {
      int status = 0;
      waitpid(child, &status, 0);
      ret = WEXITSTATUS(status);
    }
  }

  consume_args(context, command_count);
  free(terminator);

  for (i=0; i < command_count; i++) {
    free(command[i]);
  }
  free(command);
  return ret;
}
