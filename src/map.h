#pragma once

#include "vector.h"

typedef struct
{
  Vector *keys;
  Vector *data;
  size_t len;
} Map;

Map *new_map();
void put_map(Map *map, const char *key, void *value);
void *read_map(Map *map, const char *key);
int contains_map(Map *map, const char *key);
