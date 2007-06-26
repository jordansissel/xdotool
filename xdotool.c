/* xdotool
 *
 * command line interface to the xdo library
 *
 * $Id$
 * 
 */

#include <stdio.h>
#include <stdlib.h>

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
  argc -= 2;

  argv++; /* skip 'cmd' */

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

  if (argc != 2) {
    fprintf(stderr, "usage: move <xcoord> <ycoord>\n");
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

  if (argc != 1) {
    fprintf(stderr, "usage: mousedown <button>\n");
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

  if (argc != 1) {
    fprintf(stderr, "usage: mousedown <button>\n");
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

  if (argc == 0) {
    fprintf(stderr, "usage: type <things to type>\n");
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

  if (argc == 0) {
    fprintf(stderr, "usage: move <keyseq1> [keyseq2 ... keyseqN]\n");
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
  if (argc != 3) {
    printf("usage: windowmove wid x y\n");
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
  if (argc != 1) {
    printf("usage: windowfocus wid\n");
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  if (!xdo_window_focus(xdo, wid)) {
    fprintf(stderr, "xdo_window_focus reported an error\n");
  }
}

void cmd_windowraise(int argc, char **args) {
  Window wid;
  if (argc != 1) {
    printf("usage: windowraise wid\n");
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  if (!xdo_window_raise(xdo, wid)) {
    fprintf(stderr, "xdo_window_raise reported an error\n");
  }
}

void cmd_windowsize(int argc, char **args) {
  int width, height;
  Window wid;
  if (argc != 3) {
    printf("usage: windowsize wid width height\n");
    return;
  }

  wid = (int)strtol(args[0], NULL, 0);
  width = (int)strtol(args[1], NULL, 0);
  height = (int)strtol(args[2], NULL, 0);

  if (!xdo_window_setsize(xdo, wid, width, height)) {
    fprintf(stderr, "xdo_window_setsize reported an error\n");
  }
}

void cmd_search(int argc, char **args) {
  Window *list;
  int nwindows;
  int i;

  if (argc != 1) {
    printf("usage: search regex_pattern\n");
    return;
  }

  xdo_window_list_by_regex(xdo, *args, &list, &nwindows);
  for (i = 0; i < nwindows; i++) {
    printf("%d\n", list[i]);
  }

  /* Free list as it's malloc'd by xdo_window_list_by_regex */
  free(list);
}
