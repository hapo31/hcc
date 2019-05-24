#include <stddef.h>

typedef struct
{
    size_t len;
    size_t capacity;
    char *str;
} String;

String *new_string();
String *append_string(String *src, const char *str);
String *put_string(String *src, const char *str);
