#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include "rev4_matrix.h"

uint64_t mat2[1024][1024];

char secret[32] = {
    0xB3, 0x26, 0x31, 0x2D, 0xD7, 0x8D, 0x84, 0x9F, 0x96, 0xD3, 0x1F, 0xEE, 0x68, 0x72, 0x16, 0xA3, 
    0xC2, 0xC1, 0xA7, 0x1E, 0x08, 0xE6, 0xED, 0x85, 0x05, 0xB5, 0x86, 0xEF, 0xD6, 0x33, 0x1C, 0x09
};

void dumpflag(int64_t a1, const void *a2, signed int a3)
{
    int64_t v3; // rax@1
    signed int v4; // eax@4
    signed int v6; // [sp+Ch] [bp-74h]@1
    int v7; // [sp+Ch] [bp-74h]@6
    int64_t v8; // [sp+18h] [bp-68h]@1
    int i; // [sp+2Ch] [bp-54h]@6
    char v10[32]; // [sp+30h] [bp-50h]@1
    char s[40]; // [sp+50h] [bp-30h]@6

    v8 = a1;
    v6 = a3;
    if ( !SHA256(&v8, 8LL, v10) )
    {
        fwrite("something went wrong ...\n", 1uLL, 0x19uLL, stderr);
        exit(-1);
    }
    v4 = 32;
    if ( v6 <= 32 )
        v4 = v6;
    v7 = v4;
    bzero(s, 0x21uLL);
    bcopy(a2, s, v7);
    for ( i = 0; i < v7; ++i )
        s[i] ^= v10[i];
    s[i] = 0;
    fprintf(stderr, "# The flag is '%s'\n", s);
}

#define valid(a1, a2) (a1 < 0x400u && a2 < 0x400u)
#define acc(x, y) (matrix[ (x) * 1024 + (y)])

uint64_t global_value = -1;

void recursive(unsigned int a1, unsigned int a2, int64_t a4)
{
    uint64_t val = matrix[a1 * 1024 + a2] + a4;
    if ( 0x400u - 1 != a1 || 0x400u - 1 != a2 )
    {
        if ( valid(a1, a2 + 1) ) {
            recursive(a1, a2 + 1, val);
        }
        if ( valid(a1 + 1, a2) ) {
            recursive(a1 + 1, a2, val);
        }
    }
    else if ( global_value > val )
    {
        global_value = val;
    }
}

void nonrecursive()
{
    mat2[0][0] = matrix[0];

    for(int j = 1; j < 1024; j++) {
        mat2[0][j] = mat2[0][j-1] + acc(0, j);
    }

    for(int i = 1; i < 1024; i++) {
        mat2[i][0] = mat2[i - 1][0] + acc(i, 0);

        for(int j = 1; j < 1024; j++) {
            mat2[i][j] = (mat2[i - 1][j] < mat2[i][j - 1] ? mat2[i - 1][j] : mat2[i][j - 1]) + acc(i, j);
        }
    }

    global_value = mat2[1023][1023];
}

int main(int argc, const char **argv, const char **envp)
{
    nonrecursive();
    printf("# solved %lu\n", global_value);
    dumpflag(global_value, secret, 32);
    exit(0);
}