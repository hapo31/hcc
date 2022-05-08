int allocate();

int main()
{
    int *p;
    allocate(&p, 1, 2, 3);

    if (*p != 1)
    {
        return *p;
    }

    if (*(p + 1) != 2)
    {
        return *p;
    }

    if (*(p + 2) != 3)
    {
        return *p;
    }

    return 0;
}
