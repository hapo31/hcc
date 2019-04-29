#include <stdio.h>

#include "../src/vector.h"
#include "../src/map.h"

int test_count = 0;
int failed_count = 0;

void print_success()
{
  printf("\x1b[32m[SUCCESS]\x1b[0m [%d/%d] tests are passed.\n", test_count - failed_count, test_count);
}

void print_error()
{
  printf("\x1b[31m[FAILED]\x1b[0m [%d/%d] tests are failed.\n", failed_count, test_count);
}

void expect(int line, int expected, int actual)
{
  ++test_count;
  if (expected == actual)
  {
    printf("\x1b[32m[OK]\x1b[0m %d => %d\n", expected, actual);
    return;
  }
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  ++failed_count;
}

int main()
{
  /**
   * Vector のテスト
   */
  Vector *vec = new_vector(10);
  for (int i = 0; i < 100; ++i)
  {
    push_vector(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  /**
   * Map のテスト
   */

  Map *map = new_map();

  expect(__LINE__, NULL, read_map(map, "hoge"));
  expect(__LINE__, 0, contains_map(map, "hoge"));

  put_map(map, "hoge", (void *)114514);
  expect(__LINE__, 1, contains_map(map, "hoge"));
  expect(__LINE__, 114514, ((int *)read_map(map, "hoge")));

  print_success();
  if (failed_count >= 1)
  {
    print_error();
  }
}