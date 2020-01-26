#include <stdlib.h>
#include <stdio.h>

int main() {
    int seed;
    scanf("%d", &seed);
    srand(seed);
    for (int i = 0; i < 48; ++i) printf("%d,", rand() % 100);
    return 0;
}
