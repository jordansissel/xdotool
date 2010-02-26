
#include <stdlib.h>
#include <regex.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include "xdo.h"

static void _xdo_get_child_windows(const xdo_t *xdo, Window window, int max_depth,
                                   Window **candidate_window_list, 
                                   int *ncandidate_windows, 
                                   int *window_list_size);
static int compile_re(const char *pattern, regex_t *re);
static int check_window_match(const xdo_t *xdo, Window wid, const xdo_search_t *search);
static int _xdo_window_match_class(const xdo_t *xdo, Window window, regex_t *re);
static int _xdo_window_match_name(const xdo_t *xdo, Window window, regex_t *re);
static int _xdo_window_match_title(const xdo_t *xdo, Window window, regex_t *re);
static int _xdo_window_match_pid(const xdo_t *xdo, Window window, const unsigned long pid);
static int _xdo_is_window_visible(const xdo_t *xdo, Window wid);

static Atom _NET_WM_PID = -1;

int xdo_window_search(const xdo_t *xdo, const xdo_search_t *search,
                      Window **windowlist_ret, int *nwindows_ret) {
  Window *candidate_window_list = NULL;
  int ncandidate_windows = 0;
  int candidate_window_list_size = 0;
  int matched_window_list_size = 100;
  int i = 0;

  *nwindows_ret = 0;
  *windowlist_ret = calloc(sizeof(Window), matched_window_list_size);

  /* TODO(sissel): Support multiple screens */
  if (search->searchmask & SEARCH_SCREEN) {
    _xdo_get_child_windows(xdo, RootWindow(xdo->xdpy, search->screen), search->max_depth,
                           &candidate_window_list, &ncandidate_windows,
                           &candidate_window_list_size);
  } else {
    const int screencount = ScreenCount(xdo->xdpy);
    for (i = 0; i < screencount; i++) {
      _xdo_get_child_windows(xdo, RootWindow(xdo->xdpy, i), search->max_depth,
                             &candidate_window_list, &ncandidate_windows,
                             &candidate_window_list_size);
    }
  }
  //printf("Window count: %d\n", (int)ncandidate_windows);
  //printf("Search:\n");
  //printf("onlyvisible: %d\n", search->only_visible);
  //printf("pid: %lu\n", search->pid);
  //printf("title: %s\n", search->title);
  //printf("name: %s\n", search->winname);
  //printf("class: %s\n", search->winclass);
  //printf("/Search\n");
  for (i = 0; i < ncandidate_windows; i++) {
    Window wid = candidate_window_list[i];
    int result = check_window_match(xdo, wid, search);
    if (!result)
      continue;

    /* If we get this far, the window matches all our requirements */
    (*windowlist_ret)[*nwindows_ret] = wid;
    (*nwindows_ret)++;

    if (matched_window_list_size == *nwindows_ret) {
      matched_window_list_size *= 2;
      *windowlist_ret = realloc(*windowlist_ret, 
                                matched_window_list_size * sizeof(Window));
    }
  }
  free(candidate_window_list);

  return True;
}

static void _xdo_get_child_windows(const xdo_t *xdo, Window window, 
                                   int max_depth,
                                   Window **candidate_window_list, 
                                   int *ncandidate_windows,
                                   int *window_list_size) {
  Window dummy;
  Window *children;
  unsigned int i, nchildren;

  if (max_depth == 0) {
    return;
  }

  if (*window_list_size == 0) {
    *ncandidate_windows = 0;
    *window_list_size = 100;
    *candidate_window_list = malloc(*window_list_size * sizeof(Window));
  }

  if (!XQueryTree(xdo->xdpy, window, &dummy, &dummy, &children, &nchildren))
    return;

  for (i = 0; i < nchildren; i++) {
    Window w = children[i];
    (*candidate_window_list)[*ncandidate_windows] = w;
    *ncandidate_windows += 1;
    if (*ncandidate_windows == *window_list_size) {
      *window_list_size *= 2;
      *candidate_window_list = realloc(*candidate_window_list,
                                   *window_list_size * sizeof(Window));
    }
    _xdo_get_child_windows(xdo, w, max_depth - 1, candidate_window_list,
                           ncandidate_windows, window_list_size);
  }

  XFree(children);
}

static int _xdo_window_match_title(const xdo_t *xdo, Window window, regex_t *re) {
  int i;
  int count = 0;
  char **list = NULL;
  XTextProperty tp;

  XGetWMName(xdo->xdpy, window, &tp);
  if (tp.nitems > 0) {
    XmbTextPropertyToTextList(xdo->xdpy, &tp, &list, &count);
    for (i = 0; i < count; i++) {
      //printf("%d: title '%s'\n", window, list[i]);
      if (regexec(re, list[i], 0, NULL, 0) == 0) {
        XFreeStringList(list);
        XFree(tp.value);
        return True;
      }
    }
  }
  XFreeStringList(list);
  XFree(tp.value);
  return False;
}

static int _xdo_window_match_name(const xdo_t *xdo, Window window, regex_t *re) {
  XWindowAttributes attr;
  XClassHint classhint;
  XGetWindowAttributes(xdo->xdpy, window, &attr);

  if (XGetClassHint(xdo->xdpy, window, &classhint)) {
    if ((classhint.res_name) && (regexec(re, classhint.res_name, 0, NULL, 0) == 0)) {
      XFree(classhint.res_name);
      XFree(classhint.res_class);
      return True;
    }
    XFree(classhint.res_name);
    XFree(classhint.res_class);
  }
  return False;
}

static int _xdo_window_match_class(const xdo_t *xdo, Window window, regex_t *re) {
  XWindowAttributes attr;
  XClassHint classhint;
  XGetWindowAttributes(xdo->xdpy, window, &attr);

  if (XGetClassHint(xdo->xdpy, window, &classhint)) {
    //printf("%d: class %s\n", window, classhint.res_class);
    if ((classhint.res_class) && (regexec(re, classhint.res_class, 0, NULL, 0) == 0)) {
      XFree(classhint.res_name);
      XFree(classhint.res_class);
      return True;
    }
    XFree(classhint.res_name);
    XFree(classhint.res_class);
  }
  return False;
}

static int _xdo_window_match_pid(const xdo_t *xdo, Window window, const unsigned long pid) {
  /* TODO(sissel): To implement. (Bug/issue #10) */
  Atom type;
  int size;
  long nitems;
  unsigned char *data;
  unsigned long window_pid = 0;

  if (_NET_WM_PID == (Atom)-1) {
    _NET_WM_PID = XInternAtom(xdo->xdpy, "_NET_WM_PID", False);
  }

  data = xdo_getwinprop(xdo, window, _NET_WM_PID, &nitems, &type, &size);

  if (nitems > 0) {
    window_pid = *((unsigned long *)data);
  }
  free(data);

  if (pid == window_pid) {
    return True;
  } else {
    return False;
  }
}

static int compile_re(const char *pattern, regex_t *re) {
  int ret;
  if (pattern == NULL) {
    return True;
  }

  ret = regcomp(re, pattern, REG_EXTENDED | REG_ICASE);
  if (ret != 0) {
    fprintf(stderr, "Failed to compile regex (return code %d): '%s'\n", ret, pattern);
    return False;
  }
  return True;
}

static int _xdo_is_window_visible(const xdo_t *xdo, Window wid) {
  XWindowAttributes wattr;
  XGetWindowAttributes(xdo->xdpy, wid, &wattr);
  if (wattr.map_state != IsViewable)
    return False;

  return True;
}

static int check_window_match(const xdo_t *xdo, Window wid, const xdo_search_t *search) {
  regex_t title_re;
  regex_t class_re;
  regex_t name_re;

  if (!compile_re(search->title, &title_re) \
      || !compile_re(search->winclass, &class_re) \
      || !compile_re(search->winname, &name_re)) {
    return False;
  }

  /* Set this to 1 for dev debugging */
  const int debug = 0;

  int visible_ok, pid_ok, title_ok, name_ok, class_ok;
  int visible_want, pid_want, title_want, name_want, class_want;

  visible_ok = pid_ok = title_ok = name_ok = class_ok = True;
    //(search->require == SEARCH_ANY ? False : True);

  visible_want = search->searchmask & SEARCH_ONLYVISIBLE;
  pid_want = search->searchmask & SEARCH_PID;
  title_want = search->searchmask & SEARCH_TITLE;
  name_want = search->searchmask & SEARCH_NAME;
  class_want = search->searchmask & SEARCH_CLASS;
  //visible_want = search->only_visible;
  //pid_want = (search->pid > 0);
  //title_want = (search->title != NULL);
  //name_want = (search->winname != NULL);
  //class_want = (search->winclass != NULL);

  do {
    /* Visibility is a hard condition, fail always if we wanted 
     * only visible windows and this one isn't */
    if (visible_want && !_xdo_is_window_visible(xdo, wid)) {
      if (debug) fprintf(stderr, "skip %ld visible\n", wid); 
      visible_ok = False;
      break;
    }

    if (pid_want && !_xdo_window_match_pid(xdo, wid, search->pid)) {
      if (debug) fprintf(stderr, "skip %ld pid\n", wid); 
      pid_ok = False;
    }

    if (title_want && !_xdo_window_match_title(xdo, wid, &title_re)) {
      if (debug) fprintf(stderr, "skip %ld title\n", wid);
      title_ok = False;
    }

    if (name_want && !_xdo_window_match_name(xdo, wid, &name_re)) {
      if (debug) fprintf(stderr, "skip %ld winname\n", wid);
      name_ok = False;
    }

    if (class_want && !_xdo_window_match_class(xdo, wid, &class_re)) {
      if (debug) fprintf(stderr, "skip %ld winclass\n", wid);
      class_ok = False;
    }
  } while (0);

  if (search->title) 
    regfree(&title_re);
  if (search->winclass) 
    regfree(&class_re);
  if (search->winname) 
    regfree(&name_re);

  switch (search->require) {
    case SEARCH_ALL:
      if (debug) {
        fprintf(stderr, "pid:%d, title:%d, name:%d, class:%d\n",
                pid_ok, title_ok, name_ok, class_ok);
      }
      return visible_ok && pid_ok && title_ok && name_ok && class_ok;
      break;
    case SEARCH_ANY:
      if (debug) {
        fprintf(stderr, "pid:%d, title:%d, name:%d, class:%d\n",
                pid_ok, title_ok, name_ok, class_ok);
      }
      return visible_ok && ((pid_want && pid_ok) || (title_want && title_ok) \
                            || (name_want && name_ok) \
                            || (class_want && class_ok));
      break;
  }
  
  fprintf(stderr, 
          "Unexpected code reached. search->require is not valid? (%d); this may be a bug?\n",
          search->require);
  return False;
}
