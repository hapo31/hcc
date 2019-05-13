#include <stdio.h>

int f()
{
  printf("test!!!\n");
}

int g(int i, int j, int k, int l, int m, int n)
{
  printf("test: %d, %d, %d, %d, %d, %d\n", i, j, k, l, m, n);

  return i + j + k + l + m + n;
}

int g2(int i, int j, int k, int l, int m, int n, int o)
{
  printf("test: %d, %d, %d, %d, %d, %d, %d\n", i, j, k, l, m, n, o);

  return i + j + k + l + m + n + o;
}

int f1(int i, int j, int k, int l, int m, int n, int o)
{
  return i + j + k + l + m + n + o;
}

int h()
{
  return g2(1, 2, 3, 4, 5, 6, 7);
}
