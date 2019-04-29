#pragma once

typedef struct
{
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector(int capacity);
void push_vector(Vector *vec, void *elem);
