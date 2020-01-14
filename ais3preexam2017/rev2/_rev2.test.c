#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

unsigned R;

unsigned _rand() {
  R = 1103515245 * R + 12345;
  R %= (1U<<31) - 1;
  return R % (RAND_MAX);
}

int __cdecl main(int argc, const char **argv, const char **envp)
{
  printf("%u\n", RAND_MAX);


  int seed; // ST18_4@1
  FILE *v4; // ST1C_4@1
  unsigned int v5; // ST18_4@1
  int v6; // ebx@2
  int v7; // eax@2
  int result; // eax@4
  int v9; // ecx@4
  int v10; // [sp-Ch] [bp-4Ch]@0
  int v11; // [sp-8h] [bp-48h]@0
  int v12; // [sp-4h] [bp-44h]@0
  int v13; // [sp+0h] [bp-40h]@0
  signed __int32 i; // [sp+0h] [bp-40h]@1
  FILE *stream; // [sp+8h] [bp-38h]@1
  int v16; // [sp+Ch] [bp-34h]@0
  signed __int32 v17; // [sp+Ch] [bp-34h]@1
  //char s[36]; // [sp+10h] [bp-30h]@1
  int v19; // [sp+34h] [bp-Ch]@1
  int *v20; // [sp+3Ch] [bp-4h]@1

  struct tm T = {0};
  T.tm_year = 2017-1900;
  T.tm_mon = 5;
  T.tm_mday = 26;
  time_t t = mktime(&T);

  FILE* f = fopen("encrypted.1", "r");
  char src[100];
  fread(src, 1, 24, f);
  for (unsigned s=1498460400; s<1498460400+60*60*24; s++) {
    R = s;
    for ( i = 0; i < 24; ++i )
    {
      v6 = src[i];
      v7 = _rand();
      putchar(v6 ^ v7);
    }
    putchar('\n');
    
  }

  // stream = fopen("encrypted", "wb");
  // puts("Welcome to the encryption system.");
  // puts("We build up this system on 2017/6/26 (UTC+8)");
  // puts("Give me a string. I will give you an encrypted result. :D");
  // scanf("%s", s);
  // v17 = strlen(s);
  // for ( i = 0; i < v17; ++i )
  // {
  //   v6 = s[i];
  //   v7 = rand();
  //   fputc(v6 ^ v7, stream);
  // }
  // fclose(stream);
  result = 0;
  return result;
}