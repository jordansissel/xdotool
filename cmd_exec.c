#include "xdo_cmd.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int cmd_exec(context_t *context) {

  char *cmd = *context->argv;
  char **command = (char **) calloc(1, sizeof(char *));
  int command_count = 0;
  int ret = EXIT_SUCCESS;
  int opsync = 0;
  int arity = -1;
  char *terminator = calloc(1, sizeof(char));
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
    "            is to fork a child process and continue.\n";
  
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
	terminator = realloc(terminator, (strlen(optarg)+1) * sizeof(char));
	strncpy(terminator, optarg, strlen(optarg));
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

  if(arity > 0 && strlen(terminator) > 0) {
    fprintf(stderr, "Don't use both --terminator and --args.\n");
    return EXIT_FAILURE;
  }

  for (i=0; i<context->argc; i++) {

    if (arity > 0 && i == arity)
      break;
    
    if (strlen(terminator) > 0 && strcmp(terminator, context->argv[i]) == 0) {
      consume_args(context, 1);
      break;
    }
    
    command = realloc(command, (command_count+1) * sizeof(char *));
    command[command_count] = (char *) calloc(strlen(context->argv[i])+1, sizeof(char));
    strncpy(command[command_count], context->argv[i], strlen(context->argv[i])+1);      
    command_count++;
  }
  
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
  for(i=0; i<command_count; i++) {
    free(command[i]);
  }
  free(command);
  return ret;
}
