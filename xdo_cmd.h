
#ifndef _XDO_CMD_H_
#define _XDO_CMD_H_

#define _GNU_SOURCE 1
#ifndef __USE_BSD
#define __USE_BSD /* for strdup on linux/glibc */
#endif /* __USE_BSD */

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "xdo.h"
#include "xdotool.h"

extern char *PROGRAM;

/* implementation is in xdotool.c */
extern void consume_args(context_t *context, int argc);
extern void window_list(context_t *context, int window_arg,
                        Window **windowlist_ret, int *nwindows_ret,
                        int add_to_list);

#endif /* _XDO_CMD_H_ */
