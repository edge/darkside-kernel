#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif
int printf(const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
void panic(const char *fmt, ...);
#ifdef __cplusplus
}
#endif

#endif
