#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void warning(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

int is_alpha_or_num(char c)
{
    if (c >= 'A' && c <= 'Z' ||
        c >= 'a' && c <= 'z' ||
        c >= '0' && c <= '9' ||
        c == '_')
    {
        return 1;
    }

    return 0;
}
