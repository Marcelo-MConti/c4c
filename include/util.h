#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>

#define ARR_SIZE(x) sizeof((x))/sizeof(*(x))

size_t utf8len(const char *str);

#endif /* _UTIL_H_ */
