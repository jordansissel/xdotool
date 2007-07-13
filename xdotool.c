/* xdotool
 *
 * command line interface to the xdo library
 *
 * $Id$
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "xdo.h"

void cmd_mousemove(int argc, char **args);
void cmd_mousedown(int argc, char **args);
void cmd_mouseup(int argc, char **args);
void cmd_click(int argc, char **args);
void cmd_type(int argc, char **args);
void cmd_key(int argc, char **args);
void cmd_windowmove(int argc, char **args);
void cmd_windowfocus(int argc, char **args);
void cmd_windowsize(int argc, char **args);
void cmd_windowraise(int argc, char **args);
void cmd_search(int argc, char **args);

xdo_t *xdo;

struct dispatch {
  const char *name;
  void (*func)(int argc, char **args);
} dispatch[] = {
  "search", cmd_search,
  "windowsize", cmd_windowsize,
  "windowfocus", cmd_windowfocus,
  "windowraise", cmd_windowraise,
  "windowmove", cmd_windowmove,
  "mousemove", cmd_mousemove,
  "mousedown", cmd_mousedown,
  "mouseup", cmd_mouseup,
  "click", cmd_click,
  "type", cmd_type,
  "key", cmd_key,
  NULL, NULL,
};

int main(int argc, char **argv) {
  char *cmd;
  int i;

  if (argc < 2) {
    fprintf(stderr, "usage: %s <cmd> <args>\n", argv[0]);
    exit(1);
  }

  cmd = *++argv; /* argv[1] */
  argc -= 1; /* ignore arg0 (program name) */

  xdo = xdo_new(getenv("DISPLAY"));
  if (xdo == NULL) {
    fprintf(stderr, "Failed creating new xdo instance\n");
    return 1;
  }

  for (i = 0; dispatch[i].name != NULL; i++) {
    if (!strcasecmp(dispatch[i].name, cmd)) {
      dispatch[i].func(argc, argv);
      break;
    }
  }

  xdo_free(xdo);
  return 0;
}


void cmd_mousemove(int argc, char **args) {
  int x, y;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <xcoord> <ycoord>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return;
  }

  x = atoi(args[0]);
  y = atoi(args[1]);

  if (!xdo_mousemove(xdo, x, y)) {
    fprintf(stderr, "xdo_mousemove reported an error\n");
  }
}

void cmd_mousedown(int argc, char **args) {
  int button;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc != 1) {
    fprintf(stderr, "usage: %s <button>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return;
  }

  button = atoi(args[0]);

  if (!xdo_mousedown(xdo, button)) {
    fprintf(stderr, "xdo_mousedown reported an error\n");
  }
}

void cmd_mouseup(int argc, char **args) {
  int button;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc != 1) {
    fprintf(stderr, "usage: %s <button>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return;
  }

  button = atoi(args[0]);

  if (!xdo_mouseup(xdo, button)) {
    fprintf(stderr, "xdo_mouseup reported an error\n");
  }
}

void cmd_click(int argc, char **args) {
  cmd_mousedown(argc, args);
  cmd_mouseup(argc, args);
}

void cmd_type(int argc, char **args) {
  int i;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc == 0) {
    fprintf(stderr, "usage: %s <things to type>\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return;
  }

  for (i = 0; i < argc; i++) {
    if (!xdo_type(xdo, args[i])) {
      fprintf(stderr, "xdo_type reported an error\n");
    }
  }
}

void cmd_key(int argc, char **args) {
  int i;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc == 0) {
    fprintf(stderr, "usage: %s <keyseq1> [keyseq2 ... keyseqN]\n", cmd);
    fprintf(stderr, "You specified the wrong number of args.\n");
    return;
  }

  for (i = 0; i < argc; i++) {
    if (!xdo_keysequence(xdo, args[i])) {
      fprintf(stderr, "xdo_keysequence reported an error\n");
    }
  }
}

void cmd_windowmove(int argc, char **args) {
  int x, y;
  Window wid;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc != 3) {
    printf("usage: %s wid x y\n", cmd);
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  x = (int)strtol(args[1], NULL, 0);
  y = (int)strtol(args[2], NULL, 0);

  if (!xdo_window_move(xdo, wid, x, y)) {
    fprintf(stderr, "xdo_window_mvoe reported an error\n");
  }
}

void cmd_windowfocus(int argc, char **args) {
  Window wid;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  if (!xdo_window_focus(xdo, wid)) {
    fprintf(stderr, "xdo_window_focus reported an error\n");
  }
}

void cmd_windowraise(int argc, char **args) {
  Window wid;
  char *cmd = *args;
  argc -= 1;
  args++;

  if (argc != 1) {
    printf("usage: %s wid\n", cmd);
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  if (!xdo_window_raise(xdo, wid)) {
    fprintf(stderr, "xdo_window_raise reported an error\n");
  }
}

void cmd_windowsize(int argc, char **args) {
  int width, height;
  int wid;
  int c;

  int respect_increment = 0;
  struct option longopts[] = {
    { "respect_increment", 0, &respect_increment, 1 },
    { 0, 0, 0, 0 },
  };

  int size_flags = 0;
  char *cmd = *args;

  while (1) {
    int option_index;

    c = getopt_long_only(argc, args, "", longopts, &option_index);

    if (c == -1)
      break;
  }

  if (respect_increment)
    size_flags |= SIZE_RESPECTINCREMENT;

  args += optind;
  argc -= optind;

  if (argc != 3) {
    printf("usage: %s wid width height\n", cmd);
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  width = (int)strtol(args[1], NULL, 0);
  height = (int)strtol(args[2], NULL, 0);

  if (!xdo_window_setsize(xdo, wid, width, height, size_flags)) {
    fprintf(stderr, "xdo_window_setsize reported an error\n");
  }
}

void cmd_search(int argc, char **args) {
  Window *list;
  int nwindows;
  int i;
  int c;

  int only_visible = 0;
  int search_title = 0;
  int search_name = 0;
  int search_class = 0;
  struct option longopts[] = {
    { "onlyvisible", 0, &only_visible, 1 },
    { "title", 0, &search_title, 1 },
    { "name", 0, &search_name, 1 },
    { "class", 0, &search_class, 1 },
    { 0, 0, 0, 0 },
  };

  int search_flags = 0;

  char *cmd = *args;

  while (1) {
    int option_index;

    c = getopt_long_only(argc, args, "", longopts, &option_index);

    if (c == -1)
      break;
  }

  if (only_visible)
    search_flags |= SEARCH_VISIBLEONLY;
  if (search_title)
    search_flags |= SEARCH_TITLE;
  if (search_name)
    search_flags |= SEARCH_NAME;
  if (search_class)
    search_flags |= SEARCH_CLASS;

  args += optind;
  argc -= optind;

  if (argc != 1) {
    printf(
    "Usage: xdotool search "
    "[--onlyvisible] [--title --class --name] regexp_pattern\n"
    " --onlyvisible   matches only windows currently visible\n"
    " --title         check regexp_pattern agains the window title\n"
    " --class         check regexp_pattern agains the window class\n"
    " --name          check regexp_pattern agains the window name\n"
    "\n"
    "* If none of --title, --class, and --name are specified,\n"
    "the defaults are to match any of them.\n"
    );
    return;
  }

  xdo_window_list_by_regex(xdo, *args, search_flags, &list, &nwindows);
  for (i = 0; i < nwindows; i++) {
    printf("%d\n", list[i]);
  }

  /* Free list as it's malloc'd by xdo_window_list_by_regex */
  free(list);
}
