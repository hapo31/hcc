int f1();
int g2();

int main()
{
    int x;
    int y;
    x = 0;
    y = 0;
    y = f1(1 + 1, 1, 1, 1, 1, 1, 1);
    x = g2(1, 1, 1, 1, 1, 1, 1);

    return x + y;
}
