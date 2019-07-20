long long hash(char* str) {
    long long res = 5381;
    while (*str != 0) {
        // printf("%c %llu\n", *str, res);
        res = res * 33 + *str;
        str++;
    }
    return res;
}

void output_jpg(char* str, int len) {
    FILE* f = fopen("flaggggg.jpg", "w");
    if (f == NULL) return;
    if (  hash(&len) == 177593 &&
          hash(str) == 8932587927620123215LL) {
        // hash(str) == 0x7BF6ED64AEBD7A4F
        // len = 20
        int x = 0;
        for (char* ptr = jpg_start; ptr != jpg_end; ptr++) {
            x++;
            *ptr ^= str[x % len];
            fwrite(ptr, 1, 1, f);
        }
    }
    fclose(f);
}
