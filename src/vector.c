#include <stdlib.h>
#include <stdint.h>

#include "vector.h"

Vector *new_vector(int capacity)
{
    Vector *vec = (Vector *)malloc(sizeof(Vector));
    vec->data = (void *)malloc(sizeof(void *) * capacity);
    vec->capacity = capacity;
    vec->len = 0;

    return vec;
}

void push_vector(Vector *vec, void *elem)
{
    if (vec->len >= vec->capacity)
    {
        vec->capacity *= 2;
        void *tmp = realloc(vec->data, sizeof(void *) * vec->capacity);
        vec->data = tmp;
    }
    vec->data[vec->len] = elem;
    ++vec->len;
}
