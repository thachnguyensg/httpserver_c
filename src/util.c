#include <stdarg.h>
#include <stdio.h>

void debug_log(const char *__restrict __format, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, __format);
  vprintf(__format, args);
  va_end(args);
#endif
}
