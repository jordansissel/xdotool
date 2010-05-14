
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

extern xdo_t *xdo;
extern char *PROGRAM;
#endif /* _XDO_CMD_H_ */
