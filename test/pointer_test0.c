int allocate();

int main()
{
    int *p;
    allocate(&p, 1, 2, 3);

    if (*p != 1)
    {
        return 1;
    }

    if (*(p + 1) != 2)
    {
        return 1;
    }

    if (*(p + 2) != 3)
    {
        return 1;
    }

    return 0;
}
