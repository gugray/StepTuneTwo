#include "error.h"

// Global
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const size_t buf_sz = 4096;

void throwf(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list args;

    char *buf = new char[buf_sz];
    va_start(args, fmt);
    vsnprintf(buf, buf_sz, fmt, args);
    va_end(args);

    throw st_exception(buf, file, line, func);
}

void throwf_errno(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list args;

    char *buf1 = new char[buf_sz];
    va_start(args, fmt);
    vsnprintf(buf1, buf_sz, fmt, args);
    va_end(args);

    char *buf2 = new char[buf_sz];
    snprintf(buf2, buf_sz, "%s: %d: %s", buf1, errno, strerror(errno));

    throw st_exception(buf2, file, line, func);
}
