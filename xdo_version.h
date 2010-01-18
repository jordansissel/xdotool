#ifndef _VERSION_H_
#define _VERSION_H_

#ifndef BUILDVERSION
#error No BUILDVERSION defined.
#endif
static const char *XDO_VERSION = BUILDVERSION;
#endif /* ifndef _VERSION_H */
