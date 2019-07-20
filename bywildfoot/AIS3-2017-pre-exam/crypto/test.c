#include <stdio.h>

int main()
{
    char c[4] = "AIS3";
    int *ptr = (int *)c;
    printf("%d\n", 964600246 ^ ptr[0]);
    //return 0;

    //char str[24];
    //getBin(964600246 ^ ptr[0], str);
    //printf("%s\n", str);
    return 0;
}
