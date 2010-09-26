#include "xdo_cmd.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int cmd_exec(context_t *context) {
  int ret = EXIT_SUCCESS;
  char *cmd = *context->argv;
  int opsync = 0;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_sync
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] command [arg1 arg2 ...]\n"
    "--sync    - only exit when the command given finishes. The default"
    "            is to fork a child process and continue.";

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
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc == 0) {
    fprintf(stderr, "No arguments given.");
    fprintf(stderr, usage, cmd);
    return EXIT_FAILURE;
  }

  pid_t child;
  child = fork();
  if (child == 0) { /* child */
    execvp(context->argv[0], context->argv);

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

  consume_args(context, context->argc);
  return ret;
}
