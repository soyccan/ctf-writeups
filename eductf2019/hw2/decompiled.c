#include <stdio.h>

char data[1180];

int B(char* flag, char* A1) {
    // decompiled shellcode
    for (int i = 0; A[i] != '\0'; i++)
        if (A[i] != (flag[i] + 0x23) ^ 0x66)
            return 0;
    return 1;
}

int main() {
    // A1 += A2
    int p = 0;
    while (p < 1180 && data[p] != 0xf)
        p++;
    for (int i = 0; i < 32; i++)
        data[p + i] += data[p + i + 32];

    // B += seed
    p = 0;
    while (p < 1180 && data[p] != 0x45)
        p++;
    for (int i = 0; data[p + i] != 0; i++)
        data[p + i] += seed;

    *(void (*)(char*, char*))(B)(flag, A1);
}