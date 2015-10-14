
#ifndef _XDO_CMD_H_
#define _XDO_CMD_H_

#define _GNU_SOURCE 1
#ifndef __USE_BSD
#define __USE_BSD /* for strdup on linux/glibc */
#endif /* __USE_BSD */

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xdo.h"
#include "xdotool.h"

#define HELP_SEE_WINDOW_STACK \
  "If no window is given, %1 is used. See WINDOW STACK in xdotool(1)\n"
#define HELP_CHAINING_ENDS \
  "This command consumes all arguments after it, so you cannot chain\n" \
  " additional commands after it.\n"

extern void consume_args(context_t *context, int argc);
extern void window_list(context_t *context, const char *window_arg,
                        Window **windowlist_ret, int *nwindows_ret,
                        const int add_to_list);

extern void window_save(context_t *context, Window window);
extern int is_command(char *cmd);

extern int window_is_valid(context_t *context, const char *window_arg);
extern int window_get_arg(context_t *context, int min_arg, int window_arg_pos,
                          const char **window_arg);

extern void xdotool_debug(context_t *context, const char *format, ...);
extern void xdotool_output(context_t *context, const char *format, ...);

#endif /* _XDO_CMD_H_ */
