main()
{
  int x;
  int y;
  int z;
  x = 0;
  y = 0;
  z = 0;
  for (x = 0; x < 5; x = x + 1)
  {
    for (y = 0; y < 5; y = y + 1)
    {
      z = z + 1;
    }
  }

  return z;
}