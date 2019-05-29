#include <string.h>
#include <stdlib.h>

#include "str.h"

String *new_string()
{
    String *str = (String *)malloc(sizeof(String));
    str->str = (char *)malloc(sizeof(char));
    str->str[0] = '\0';
    str->capacity = 1;
    str->len = strlen(str->str);

    return str;
}

String *append_string(String *src, const char *str)
{
    if (src->len >= src->capacity)
    {
        src->capacity += strlen(str) + 1;
        void *tmp = realloc(src->str, sizeof(char *) * src->capacity);
        src->str = tmp;
    }

    strncat(src->str, str, src->capacity);
    src->len = strlen(src->str);
    return src;
}

String *put_string(String *src, const char *str)
{
    size_t len = strlen(str);

    src = new_string();
    free(src->str);
    src->str = (char *)malloc(sizeof(char) * len + 1);
    strncpy(src->str, str, len);
    return src;
}
