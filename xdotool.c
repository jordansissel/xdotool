/* xdotool
 *
 * command line interface to the xdo library
 *
 * getwindowfocus contributed by Lee Pumphret
 * keyup/down contributed by Lee Pumphret
 *
 * XXX: Need to use 'Window' instead of 'int' where appropriate.
 * vim:expandtab shiftwidth=2 softtabstop=2
 */

#define _GNU_SOURCE 1
#ifndef __USE_BSD
#define __USE_BSD /* for strdup on linux/glibc */
#endif /* __USE_BSD */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "xdo.h"
#include "xdotool.h"

xdo_t *xdo;
char *PROGRAM;
static int script_main(int argc, char **argv);
static int args_main(int argc, char **argv);

struct dispatch {
  const char *name;
  int (*func)(int argc, char **args);
} dispatch[] = {
  /* Query functions */
  { "getactivewindow", cmd_getactivewindow, },
  { "getwindowfocus", cmd_getwindowfocus, },
  { "getwindowpid", cmd_getwindowpid, },
  { "search", cmd_search, },

  /* Help me! */
  { "help", cmd_help, },
  { "version", cmd_version, },

  /* Action functions */
  { "click", cmd_click, },
  { "getmouselocation", cmd_getmouselocation, },
  { "key", cmd_key, },
  { "keydown", cmd_key, },
  { "keyup", cmd_key, },
  { "mousedown", cmd_mousedown, },
  { "mousemove", cmd_mousemove, },
  { "mousemove_relative", cmd_mousemove_relative, },
  { "mouseup", cmd_mouseup, },
  { "type", cmd_type, },
  { "windowactivate", cmd_windowactivate, },
  { "windowfocus", cmd_windowfocus, },
  { "windowmap", cmd_windowmap, },
  { "windowmove", cmd_windowmove, },
  { "windowraise", cmd_windowraise, },
  { "windowsize", cmd_windowsize, },
  { "windowunmap", cmd_windowunmap, },

  { "set_window", cmd_set_window, },

  { "set_num_desktops", cmd_set_num_desktops, },
  { "get_num_desktops", cmd_get_num_desktops, },
  { "set_desktop", cmd_set_desktop, },
  { "get_desktop", cmd_get_desktop, },
  { "set_desktop_for_window", cmd_set_desktop_for_window, },
  { "get_desktop_for_window", cmd_get_desktop_for_window, },
  { NULL, NULL, },
};

int main(int argc, char **argv) {
  /* read stdin if stdin is not a tty or first argument is "-" */

  int want_script;
  struct stat data;
  int stat_ret;

  /* If we are being run from a script with shebang line #!/path/to/xdotool
   * then the input file will be argv[argc - 1].
   */
  stat_ret = stat(argv[argc - 1], &data);

  want_script = ((argc == 2 && !strcmp(argv[1], "-"))
                 || (argc == 2 && stat_ret == 0));
  if (want_script) {
    return script_main(argc, argv);
  } else {
    return args_main(argc, argv);
  }
}

int script_main(int argc, char **argv) {
  FILE *input = NULL;
  const char *path = argv[argc - 1];
  char *cmd;
  char buffer[4096];

  if (!strcmp(path, "-") || !isatty(0)) {
    input = fdopen(0, "r");
  } else {
    input = fopen(path, "r");
    if (input == NULL) {
      fprintf(stderr, "Failure opening '%s': %s\n", path, strerror(errno));
      return EXIT_FAILURE;
    }
  }

  int ret;
  while (fgets(buffer, 4096, input) != NULL) {
    char *line = buffer;
    // Ignore leading whitespace
    line += strspn(line, " \t");

    // blanklines or line comment are ignored, too
    if (line[0] == '\n' || line[0] == '#') {
      continue;
    }
    line[strlen(line) - 1] = '\0'; /* replace newline with null */

    if (asprintf(&cmd, "%s %s", argv[0], line) == -1) {
      fprintf(stderr, "asprintf failed\n");
      exit(1);
    }
    //printf("Running: %s\n", cmd);
    ret = system(cmd);
    free(cmd);

    if (ret != EXIT_SUCCESS) {
      return ret;
    }
  }

  return EXIT_SUCCESS;
}

int args_main(int argc, char **argv) {
  char *cmd;
  int ret = 0;
  int cmd_found = 0;
  int i;
  int opt;
  int option_index;
  const char *usage = "Usage: %s <cmd> <args>\n";
  static struct option long_options[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) {
    fprintf(stderr, usage, argv[0]);
    cmd_help(0, NULL);
    exit(1);
  }

  while ((opt = getopt_long_only(argc, argv, "+h", long_options, &option_index)) != -1) {
    switch (opt) {
      case 'h':
        cmd_help(0, NULL);
        exit(EXIT_SUCCESS);
      case 'v':
        cmd_version(0, NULL);
        exit(EXIT_SUCCESS);
      default:
        fprintf(stderr, usage, argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  PROGRAM = *argv;
  argv++; argc--;
  cmd = *argv; /* argv[1] */

  xdo = xdo_new(getenv("DISPLAY"));
  if (xdo == NULL) {
    fprintf(stderr, "Failed creating new xdo instance\n");
    return 1;
  }

  for (i = 0; dispatch[i].name != NULL && !cmd_found; i++) {
    if (!strcasecmp(dispatch[i].name, cmd)) {
      ret = dispatch[i].func(argc, argv);
      cmd_found = 1;
    }
  }

  if (!cmd_found) {
    fprintf(stderr, "%s: Unknown command: %s\n", strrchr(PROGRAM, '/') + 1, cmd);
    fprintf(stderr, "Run '%s help' if you want a command list\n", PROGRAM);
    ret = 1;
  }

  xdo_free(xdo);
  return ret;
}

int cmd_help(int unused_argc, char **unused_args) {
  int i;
  printf("Available commands:\n");
  for (i = 0; dispatch[i].name != NULL; i++)
    printf("  %s\n", dispatch[i].name);

  return 0;
}

int cmd_version(int unused_argc, char **unused_args) {
  printf("xdotool version %s\n", xdo_version());
  return 0;
}
