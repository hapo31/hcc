#include <stdlib.h>
#include <string.h>

#include "map.h"

Map *new_map()
{
    Map *map = (Map *)malloc(sizeof(Map));
    map->keys = new_vector(10);
    map->data = new_vector(10);
    map->len = 0;

    return map;
}

int contains_map(Map *map, const char *key)
{
    const size_t key_len = strlen(key);
    for (int i = 0; i < map->keys->len; ++i)
    {
        if (strncmp((const char *)map->keys->data[i], key, key_len) == 0)
        {
            return 1;
        }
    }

    return 0;
}

void put_map(Map *map, const char *key, void *value)
{
    const size_t key_len = strlen(key);
    int key_pos = -1;
    if (!contains_map(map, key))
    {
        char *key_buf = (char *)malloc(sizeof(char) * key_len + 1);
        strncpy(key_buf, key, key_len);
        key_buf[key_len] = '\0';
        push_vector(map->keys, key_buf);
        push_vector(map->data, value);
        ++map->len;
    }
    else
    {
        for (int i = 0; i < map->keys->len; ++i)
        {
            if (strncmp((const char *)map->keys->data[i], key, key_len) == 0)
            {
                map->data->data[i] = value;
                return;
            }
        }
    }
}

void *read_map(Map *map, const char *key)
{
    const size_t key_len = strlen(key);
    for (int i = 0; i < map->keys->len; ++i)
    {
        if (strncmp((const char *)map->keys->data[i], key, key_len) == 0)
        {
            return map->data->data[i];
        }
    }
    return NULL;
}
