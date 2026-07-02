/* xdo dnd implementation
 *
 * Lets you simulate drag-and-drop events
 */

// #include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
#include <X11/Xatom.h>

#include "xdo.h"
#include "xdotool.h"

// Chromium is a bit subborn, so we try multiple times
#define MAX_DND_POSITION_DENIED_COUNT 3

Time _get_x11_server_time(const xdo_t *xdo, Window window);
bool _send_xdnd_position(const xdo_t *xdo, Window target, Window window,
                        Time timestamp, int x, int y);
int _process_events_until_done(const xdo_t *xdo, Atom *format_atoms, char **values,
                               int values_count, Window target,
                               Window window, int x, int y);

Time _get_x11_server_time(const xdo_t *xdo, Window window) {
  XSelectInput(xdo->xdpy, window, PropertyChangeMask);
  XChangeProperty(xdo->xdpy, window, XA_WM_NAME, 8, 8, PropModeAppend,
                  None, 0);

  XEvent event;
  XWindowEvent(xdo->xdpy, window, PropertyChangeMask, &event);
  XSelectInput(xdo->xdpy, window, NoEventMask);
  return event.xproperty.time;
}

bool _send_xdnd_position(const xdo_t *xdo, Window target, Window window,
                        Time timestamp, int x, int y) {
  Atom XdndPosition = XInternAtom(xdo->xdpy, "XdndPosition", False);
  Atom XdndActionCopy =
      XInternAtom(xdo->xdpy, "XdndActionCopy", False);

  XEvent message;
  memset(&message, 0, sizeof(message));
  message.xclient.type = ClientMessage;
  message.xclient.display = xdo->xdpy;
  message.xclient.window = target;
  message.xclient.message_type = XdndPosition;
  message.xclient.format = 32;
  message.xclient.data.l[0] = window;
  // message.xclient.data.l[1] is reserved
  message.xclient.data.l[2] = x << 16 | y;
  message.xclient.data.l[3] = timestamp;
  message.xclient.data.l[4] = XdndActionCopy;

  if (XSendEvent(xdo->xdpy, target, False, 0, &message) == 1)
    return XDO_SUCCESS;
  return XDO_ERROR;
}

int _process_events_until_done(const xdo_t *xdo, Atom *format_atoms, char **values,
                               int values_count, Window target,
                               Window window, int x, int y) {
  /* Set this to 1 for dev debugging */
  static const int debug = 0;

  Atom XdndStatus = XInternAtom(xdo->xdpy, "XdndStatus", False);
  Atom XdndFinished = XInternAtom(xdo->xdpy, "XdndFinished", False);
  Atom TARGETS = XInternAtom(xdo->xdpy, "TARGETS", False);

  XEvent message;
  bool running = True;
  time_t start = time(NULL);
  int position_denied_count = 0;
  while (running) {
    XEvent event;
    XNextEvent(xdo->xdpy, &event);

    if (event.type == SelectionRequest) {
      XSelectionRequestEvent selection_request = event.xselectionrequest;

      if (selection_request.target == TARGETS) {
        if (debug) fprintf(stderr, "Target requested the supported target atoms");
        XChangeProperty(xdo->xdpy, selection_request.requestor,
                        selection_request.property, XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)format_atoms,
                        values);
        XChangeProperty(xdo->xdpy, selection_request.requestor,
                        selection_request.property, XA_ATOM, 32, PropModeAppend,
                        (void *)(Atom[1]){
                            TARGETS,
                        },
                        1);
        XSync(xdo->xdpy, False);

        memset(&message, 0, sizeof(message));
        message.xselection.type = SelectionNotify;
        message.xselection.display = xdo->xdpy;
        message.xselection.requestor = selection_request.requestor;
        message.xselection.selection = selection_request.selection;
        message.xselection.target = selection_request.target;
        message.xselection.property = selection_request.property;
        message.xselection.time = selection_request.time;

        if (XSendEvent(xdo->xdpy, selection_request.requestor, False,
                       0, &message) == 0) {
          fprintf(stderr,
                  "Failed to send SelectionNotify message to target.\n");
          return XDO_ERROR;
        }
      } else {
        // Send value to window
        char *to_send = NULL;
        for (int i = 0; i < values_count; i++) {
          if (format_atoms[i] == selection_request.target) {
            to_send = values[i];
          }
        }
        if (to_send == NULL) {
          fprintf(
              stderr, "Target requested an unexpected format atom %ld: %s",
              selection_request.target,
              XGetAtomName(xdo->xdpy, selection_request.target));
        } else {
          if (debug) fprintf(
              stderr, "Target requested the supported format atom %ld: %s",
              selection_request.target,
              XGetAtomName(xdo->xdpy, selection_request.target));
          // Set property on target window - do not copy end null byte
          XChangeProperty(xdo->xdpy, selection_request.requestor,
                          selection_request.property, selection_request.target,
                          8, PropModeReplace, (unsigned char *)to_send,
                          strlen(to_send));
          if (debug) fprintf(stderr, "Set property %ld of target %ld to value %s",
                        selection_request.target, selection_request.property,
                        to_send);

          memset(&message, 0, sizeof(message));
          message.xselection.type = SelectionNotify;
          message.xselection.display = xdo->xdpy;
          message.xselection.requestor = selection_request.requestor;
          message.xselection.selection = selection_request.selection;
          message.xselection.target = selection_request.target;
          message.xselection.property = selection_request.property;
          message.xselection.time = selection_request.time;

          if (XSendEvent(xdo->xdpy, selection_request.requestor, False,
                         0, &message) == 0) {
            fprintf(stderr,
                    "Failed to send SelectionNotify message to target.\n");
            return XDO_ERROR;
          }
        }
      }
    } else if (event.type == ClientMessage) {
      if (event.xclient.message_type == XdndStatus) {
        if (debug) fprintf(stderr, "Received XdndStatus client message");
        if ((event.xclient.data.l[1] & 0x1) == 1) {
          Atom XdndDrop = XInternAtom(xdo->xdpy, "XdndDrop", False);
          if (debug) fprintf(
              stderr, "Target sent back an XdndStatus and accepted the drop.");
          memset(&message, 0, sizeof(message));
          message.xclient.type = ClientMessage;
          message.xclient.display = xdo->xdpy;
          message.xclient.window = target;
          message.xclient.message_type = XdndDrop;
          message.xclient.format = 32;
          message.xclient.data.l[0] = window;
          // message.xclient.data.l[1] reserved
          message.xclient.data.l[2] = _get_x11_server_time(xdo, window);

          // Send it to target window
          if (XSendEvent(xdo->xdpy, target, False, 0, &message) == 0) {
            fprintf(stderr, "Failed to send XdndDrop message to target.\n");
          }
        } else {
          if (debug) fprintf(stderr, "Client denied our position (total %d)",
                        position_denied_count + 1);

          if (position_denied_count < MAX_DND_POSITION_DENIED_COUNT) {
            position_denied_count += 1;
            usleep(100000); /* 100ms */
            if (!_send_xdnd_position(xdo, target, window,
                                    _get_x11_server_time(xdo, window), x,
                                    y)) {
              fprintf(stderr,
                      "Failed to send XdndPosition mesage to target.\n");
              return XDO_ERROR;
            }
          } else {
            fprintf(stderr, "Target denied our XdndPosition events too many "
                            "times. Sending XdndLeave and exiting.\n");

            Atom XdndLeave =
                XInternAtom(xdo->xdpy, "XdndLeave", False);
            memset(&message, 0, sizeof(message));
            message.xclient.type = ClientMessage;
            message.xclient.display = xdo->xdpy;
            message.xclient.window = target;
            message.xclient.message_type = XdndLeave;
            message.xclient.format = 32;
            message.xclient.data.l[0] = window;
            // Other fields are reserved

            if (XSendEvent(xdo->xdpy, target, False, 0, &message) == 0)
              fprintf(stderr, "Failed to send XdndLeave message to target.\n");
            return XDO_ERROR;
          }
        }
      } else if (event.xclient.message_type == XdndFinished) {
        if (debug) fprintf(stderr, "Received XdndFinished client message");

        if ((event.xclient.data.l[1] & 0x1) == 1) {
          if (debug) fprintf(stderr,
                        "Target sent back a successful XdndFinished message");
          return XDO_SUCCESS;
        } else {
          fprintf(stderr,
                  "Target sent back an unsucessful XdndFinished message.\n");
          return XDO_ERROR;
        }
      } else {
        if (debug) fprintf(stderr, "Ignoring client message of type %ld",
                      event.xclient.message_type);
      }
    } else {
      if (debug) fprintf(stderr, "Ignoring event of type %d", event.type);
    }

    if (time(NULL) - start > 5) {
      fprintf(stderr, "Did not receive XdndStatus from target in time (5s).\n");
      return XDO_ERROR;
    }
  }
  return XDO_ERROR;
}

int xdo_drop(const xdo_t *xdo, Window target, int x, int y, char **formats,
             char **values, int values_count) {
  /* Set this to 1 for dev debugging */
  static const int debug = 0;

  // Check if target window is XdndAware
  unsigned char *property_value;
  int ret = xdo_get_window_property(xdo, target, "XdndAware",
                                &property_value, NULL, NULL, NULL);
  if (ret == XDO_ERROR) {
    fprintf(stderr,
            "The target window does not support the drag-and-drop protocol.\n");
    return XDO_ERROR;
  }
  unsigned char version = property_value[0];
  fprintf(
      stderr,
      "Target window implements the version %d of the drag-and-drop protocol.",
      version);
  if (version < 2) {
    fprintf(stderr,
            "The target window is using an obselete version (%d) of the "
            "drag-and-drop protocol.\n",
            version);
    return XDO_ERROR;
  } else if (version > 5) {
    fprintf(stderr,
            "The target window is using an usupported version (%d) of the "
            "drag-and-drop protocol.\n",
            version);
    return XDO_ERROR;
  }

  // Fetch atoms for the given formats
  Atom *format_atoms = calloc(values_count, sizeof(Atom));
  XInternAtoms(xdo->xdpy, formats, values_count, true, format_atoms);

  // Create a XdndAware window to receive events
  Window window = XCreateSimpleWindow(
      xdo->xdpy,
      RootWindow(xdo->xdpy, DefaultScreen(xdo->xdpy)), 10, 10,
      10, 10, 1,
      BlackPixel(xdo->xdpy, DefaultScreen(xdo->xdpy)),
      WhitePixel(xdo->xdpy, DefaultScreen(xdo->xdpy)));
  Atom XdndAware = XInternAtom(xdo->xdpy, "XdndAware", False);
  Atom XdndTypeList = XInternAtom(xdo->xdpy, "XdndTypeList", False);
  unsigned char source_version = 5;
  XChangeProperty(xdo->xdpy, window, XdndAware, 4, 32, PropModeReplace,
                  &source_version, 1);
  XChangeProperty(xdo->xdpy, window, XdndTypeList, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)format_atoms, values_count);
  XMapWindow(xdo->xdpy, window);
  if (debug) fprintf(stderr, "Created dnd aware source window");

  // Claim ownership of XdndSelection
  Atom XdndSelection = XInternAtom(xdo->xdpy, "XdndSelection", False);
  XSetSelectionOwner(xdo->xdpy, XdndSelection, window,
                     _get_x11_server_time(xdo, window));
  XFlush(xdo->xdpy);
  Window selection_owner =
      XGetSelectionOwner(xdo->xdpy, XdndSelection);
  if (selection_owner != window) {
    fprintf(stderr,
            "Failed to claim ownership of selection. Selection owner: %ld.\n",
            selection_owner);
    return XDO_ERROR;
  }

  if (debug) fprintf(stderr, "Claimed ownership of selection");

  // Send enter XdndEnter
  Atom XdndEnter = XInternAtom(xdo->xdpy, "XdndEnter", False);
  XEvent message;
  memset(&message, 0, sizeof(message));
  message.xclient.type = ClientMessage;
  message.xclient.display = xdo->xdpy;
  message.xclient.window = target;
  message.xclient.message_type = XdndEnter;
  message.xclient.format = 32;
  message.xclient.data.l[0] = window;
  message.xclient.data.l[1] = 5 << 24; // Xdnd version
  message.xclient.data.l[2] = values_count > 0 ? format_atoms[0] : None;
  message.xclient.data.l[3] = values_count > 1 ? format_atoms[1] : None;
  message.xclient.data.l[4] = values_count > 2 ? format_atoms[2] : None;

  if (XSendEvent(xdo->xdpy, target, False, 0, &message) == 0) {
    fprintf(stderr, "Failed to send XdndEnter message to target.\n");
    return XDO_ERROR;
  }

  if (debug) fprintf(stderr, "Sent XdndEnter to target");

  // Send XdndPosition
  if (!_send_xdnd_position(xdo, target, window,
                          _get_x11_server_time(xdo, window), x, y)) {
    fprintf(stderr, "Failed to send XdndPosition message to target.\n");
    return XDO_ERROR;
  }

  // Process events until XdndFinished is received (max 5s)
  bool accepted = _process_events_until_done(
      xdo, format_atoms, values, values_count, target, window, x, y);

  // Cleanup window
  XDestroyWindow(xdo->xdpy, window);

  return accepted ? XDO_SUCCESS : XDO_ERROR;
}
