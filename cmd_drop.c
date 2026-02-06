#include "xdo_cmd.h"
#include <X11/Xatom.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

// Chromium is a bit subborn, so we try multiple times
#define MAX_DND_POSITION_DENIED_COUNT 3

Time get_x11_server_time(context_t *context, Window window) {
  XSelectInput(context->xdo->xdpy, window, PropertyChangeMask);
  XChangeProperty(context->xdo->xdpy, window, XA_WM_NAME, 8, 8, PropModeAppend,
                  None, 0);

  XEvent event;
  XWindowEvent(context->xdo->xdpy, window, PropertyChangeMask, &event);
  XSelectInput(context->xdo->xdpy, window, NoEventMask);
  return event.xproperty.time;
}

bool send_xdnd_position(context_t *context, Window target, Window window,
                        Time timestamp, int x, int y) {
  Atom XdndPosition = XInternAtom(context->xdo->xdpy, "XdndPosition", False);
  Atom XdndActionCopy =
      XInternAtom(context->xdo->xdpy, "XdndActionCopy", False);

  XEvent message;
  memset(&message, 0, sizeof(message));
  message.xclient.type = ClientMessage;
  message.xclient.display = context->xdo->xdpy;
  message.xclient.window = target;
  message.xclient.message_type = XdndPosition;
  message.xclient.format = 32;
  message.xclient.data.l[0] = window;
  // message.xclient.data.l[1] is reserved
  message.xclient.data.l[2] = x << 16 | y;
  message.xclient.data.l[3] = timestamp;
  message.xclient.data.l[4] = XdndActionCopy;

  return XSendEvent(context->xdo->xdpy, target, False, 0, &message) == 1;
}

bool process_events_until_done(context_t *context, Atom *formats,
                               char **command, int command_count, Window target,
                               Window window, int x, int y) {
  Atom XdndStatus = XInternAtom(context->xdo->xdpy, "XdndStatus", False);
  Atom XdndFinished = XInternAtom(context->xdo->xdpy, "XdndFinished", False);
  Atom TARGETS = XInternAtom(context->xdo->xdpy, "TARGETS", False);

  XEvent message;
  bool running = True;
  time_t start = time(NULL);
  int position_denied_count = 0;
  while (running) {
    XEvent event;
    XNextEvent(context->xdo->xdpy, &event);

    if (event.type == SelectionRequest) {
      XSelectionRequestEvent selection_request = event.xselectionrequest;

      if (selection_request.target == TARGETS) {
        xdotool_debug(context, "Target requested the supported target atoms");
        XChangeProperty(context->xdo->xdpy, selection_request.requestor,
                        selection_request.property, XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)formats,
                        command_count / 2);
        XChangeProperty(context->xdo->xdpy, selection_request.requestor,
                        selection_request.property, XA_ATOM, 32, PropModeAppend,
                        (void *)(Atom[1]){
                            TARGETS,
                        },
                        1);
        XSync(context->xdo->xdpy, False);

        memset(&message, 0, sizeof(message));
        message.xselection.type = SelectionNotify;
        message.xselection.display = context->xdo->xdpy;
        message.xselection.requestor = selection_request.requestor;
        message.xselection.selection = selection_request.selection;
        message.xselection.target = selection_request.target;
        message.xselection.property = selection_request.property;
        message.xselection.time = selection_request.time;

        if (XSendEvent(context->xdo->xdpy, selection_request.requestor, False,
                       0, &message) == 0) {
          fprintf(stderr,
                  "Failed to send SelectionNotify message to target.\n");
          return False;
        }
      } else {
        // Send value to window
        char *to_send = NULL;
        for (int i = 0; i < command_count; i += 2) {
          if (formats[i / 2] == selection_request.target) {
            to_send = command[i + 1];
          }
        }
        if (to_send == NULL) {
          xdotool_debug(
              context, "Target requested an unexpected format atom %ld: %s",
              selection_request.target,
              XGetAtomName(context->xdo->xdpy, selection_request.target));
        } else {
          xdotool_debug(
              context, "Target requested the supported format atom %ld: %s",
              selection_request.target,
              XGetAtomName(context->xdo->xdpy, selection_request.target));
          // Set property on target window - do not copy end null byte
          XChangeProperty(context->xdo->xdpy, selection_request.requestor,
                          selection_request.property, selection_request.target,
                          8, PropModeReplace, (unsigned char *)to_send,
                          strlen(to_send));
          xdotool_debug(context, "Set property %ld of target %ld to value %s",
                        selection_request.target, selection_request.property,
                        to_send);

          memset(&message, 0, sizeof(message));
          message.xselection.type = SelectionNotify;
          message.xselection.display = context->xdo->xdpy;
          message.xselection.requestor = selection_request.requestor;
          message.xselection.selection = selection_request.selection;
          message.xselection.target = selection_request.target;
          message.xselection.property = selection_request.property;
          message.xselection.time = selection_request.time;

          if (XSendEvent(context->xdo->xdpy, selection_request.requestor, False,
                         0, &message) == 0) {
            fprintf(stderr,
                    "Failed to send SelectionNotify message to target.\n");
            return False;
          }
        }
      }
    } else if (event.type == ClientMessage) {
      if (event.xclient.message_type == XdndStatus) {
        xdotool_debug(context, "Received XdndStatus client message");
        if ((event.xclient.data.l[1] & 0x1) == 1) {
          Atom XdndDrop = XInternAtom(context->xdo->xdpy, "XdndDrop", False);
          xdotool_debug(
              context, "Target sent back an XdndStatus and accepted the drop.");
          memset(&message, 0, sizeof(message));
          message.xclient.type = ClientMessage;
          message.xclient.display = context->xdo->xdpy;
          message.xclient.window = target;
          message.xclient.message_type = XdndDrop;
          message.xclient.format = 32;
          message.xclient.data.l[0] = window;
          // message.xclient.data.l[1] reserved
          message.xclient.data.l[2] = get_x11_server_time(context, window);

          // Send it to target window
          if (XSendEvent(context->xdo->xdpy, target, False, 0, &message) == 0) {
            fprintf(stderr, "Failed to send XdndDrop message to target.\n");
          }
        } else {
          xdotool_debug(context, "Client denied our position (total %d)",
                        position_denied_count + 1);

          if (position_denied_count < MAX_DND_POSITION_DENIED_COUNT) {
            position_denied_count += 1;
            usleep(100000); /* 100ms */
            if (!send_xdnd_position(context, target, window,
                                    get_x11_server_time(context, window), x,
                                    y)) {
              fprintf(stderr,
                      "Failed to send XdndPosition mesage to target.\n");
              return False;
            }
          } else {
            fprintf(stderr, "Target denied our XdndPosition events too many "
                            "times. Sending XdndLeave and exiting.\n");

            Atom XdndLeave =
                XInternAtom(context->xdo->xdpy, "XdndLeave", False);
            memset(&message, 0, sizeof(message));
            message.xclient.type = ClientMessage;
            message.xclient.display = context->xdo->xdpy;
            message.xclient.window = target;
            message.xclient.message_type = XdndLeave;
            message.xclient.format = 32;
            message.xclient.data.l[0] = window;
            // Other fields are reserved

            if (XSendEvent(context->xdo->xdpy, target, False, 0, &message) == 0)
              fprintf(stderr, "Failed to send XdndLeave message to target.\n");
            return False;
          }
        }
      } else if (event.xclient.message_type == XdndFinished) {
        xdotool_debug(context, "Received XdndFinished client message");

        if ((event.xclient.data.l[1] & 0x1) == 1) {
          xdotool_debug(context,
                        "Target sent back a successful XdndFinished message");
          return True;
        } else {
          fprintf(stderr,
                  "Target sent back an unsucessful XdndFinished message.\n");
          return False;
        }
      } else {
        xdotool_debug(context, "Ignoring client message of type %d",
                      event.xclient.message_type);
      }
    } else {
      xdotool_debug(context, "Ignoring event of type %d", event.type);
    }

    if (time(NULL) - start > 5) {
      fprintf(stderr, "Did not receive XdndStatus from target in time (5s).\n");
      return False;
    }
  }
  return False;
}

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

  // Get format atoms
  Atom *formats = malloc(sizeof(Atom) * command_count / 2);
  for (i = 0; i < command_count; i += 2) {
    xdotool_debug(context, "Getting atom for format %s", command[i]);
    formats[i / 2] = XInternAtom(context->xdo->xdpy, command[i], False);
    xdotool_debug(context, "Atom for format %s: %ld", command[i],
                  formats[i / 2]);
  }

  // Find window under cursor
  int x;
  int y;
  Window target;
  ret = xdo_get_mouse_location2(context->xdo, &x, &y, NULL, &target);
  xdotool_debug(context, "Target window (at coordinates %d,%d) has for id %d",
                x, y, target);

  // Check if target window is XdndAware
  unsigned char *property_value;
  ret = xdo_get_window_property(context->xdo, target, "XdndAware",
                                &property_value, NULL, NULL, NULL);
  if (ret == XDO_ERROR) {
    fprintf(stderr,
            "The target window does not support the drag-and-drop protocol.\n");
    return EXIT_FAILURE;
  }
  unsigned char version = property_value[0];
  xdotool_debug(
      context,
      "Target window implements the version %d of the drag-and-drop protocol.",
      version);
  if (version < 2) {
    fprintf(stderr,
            "The target window is using an obselete version (%d) of the "
            "drag-and-drop protocol.\n",
            version);
    return EXIT_FAILURE;
  } else if (version > 5) {
    fprintf(stderr,
            "The target window is using an usupported version (%d) of the "
            "drag-and-drop protocol.\n",
            version);
    return EXIT_FAILURE;
  }

  // Create a XdndAware window to receive events
  Window window = XCreateSimpleWindow(
      context->xdo->xdpy,
      RootWindow(context->xdo->xdpy, DefaultScreen(context->xdo->xdpy)), 10, 10,
      10, 10, 1,
      BlackPixel(context->xdo->xdpy, DefaultScreen(context->xdo->xdpy)),
      WhitePixel(context->xdo->xdpy, DefaultScreen(context->xdo->xdpy)));
  Atom XdndAware = XInternAtom(context->xdo->xdpy, "XdndAware", False);
  Atom XdndTypeList = XInternAtom(context->xdo->xdpy, "XdndTypeList", False);
  unsigned char source_version = 5;
  XChangeProperty(context->xdo->xdpy, window, XdndAware, 4, 32, PropModeReplace,
                  &source_version, 1);
  XChangeProperty(context->xdo->xdpy, window, XdndTypeList, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)formats, command_count / 2);
  XMapWindow(context->xdo->xdpy, window);
  xdotool_debug(context, "Created dnd aware source window");

  // Claim ownership of XdndSelection
  Atom XdndSelection = XInternAtom(context->xdo->xdpy, "XdndSelection", False);
  XSetSelectionOwner(context->xdo->xdpy, XdndSelection, window,
                     get_x11_server_time(context, window));
  XFlush(context->xdo->xdpy);
  Window selection_owner =
      XGetSelectionOwner(context->xdo->xdpy, XdndSelection);
  if (selection_owner != window) {
    fprintf(stderr,
            "Failed to claim ownership of selection. Selection owner: %ld.\n",
            selection_owner);
    return EXIT_FAILURE;
  }

  xdotool_debug(context, "Claimed ownership of selection");

  // Send enter XdndEnter
  Atom XdndEnter = XInternAtom(context->xdo->xdpy, "XdndEnter", False);
  XEvent message;
  memset(&message, 0, sizeof(message));
  message.xclient.type = ClientMessage;
  message.xclient.display = context->xdo->xdpy;
  message.xclient.window = target;
  message.xclient.message_type = XdndEnter;
  message.xclient.format = 32;
  message.xclient.data.l[0] = window;
  message.xclient.data.l[1] = 5 << 24; // Xdnd version
  message.xclient.data.l[2] = command_count > 0 ? formats[0] : None;
  message.xclient.data.l[3] = command_count > 2 ? formats[1] : None;
  message.xclient.data.l[4] = command_count > 4 ? formats[2] : None;

  if (XSendEvent(context->xdo->xdpy, target, False, 0, &message) == 0) {
    fprintf(stderr, "Failed to send XdndEnter message to target.\n");
    return EXIT_FAILURE;
  }

  xdotool_debug(context, "Sent XdndEnter to target");

  // Send XdndPosition
  if (!send_xdnd_position(context, target, window,
                          get_x11_server_time(context, window), x, y)) {
    fprintf(stderr, "Failed to send XdndPosition message to target.\n");
    return EXIT_FAILURE;
  }

  // Process events until XdndFinished is received (max 5s)
  bool accepted = process_events_until_done(
      context, formats, command, command_count, target, window, x, y);
  ret = accepted ? EXIT_SUCCESS : EXIT_FAILURE;

  // Cleanup window
  XDestroyWindow(context->xdo->xdpy, window);

  free(formats);
  consume_args(context, command_count);
  free(terminator);

  for (i = 0; i < command_count; i++) {
    free(command[i]);
  }
  free(command);
  return ret;
}
