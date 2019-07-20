#include <stdio.h>
#include <stdlib.h>

int print_flag(int a1)
{
  unsigned int v1; // eax@1
  float *v2; // ebx@4
  float v3; // ST1C_4@4
  int v5[8]; // [sp+24h] [bp-34h]@2
  char *nptr; // [sp+44h] [bp-14h]@1


  int *v7; // [sp+48h] [bp-10h]@3
  int i; // [sp+4Ch] [bp-Ch]@3

  char str1[] = "3577981885800385511167897945243648 13089153870598552682496 223706287589202141406608389832704 52964205115322019086336 20304841948673309171436136945938333696"; 

  nptr = str1;
  v1 = 0;
  do
  {
    v5[v1] = 0;
    ++v1;
  }
  while ( v1 < 8 );
  v7 = v5;
  printf("AIS3");
  for ( i = 0; i <= 4; ++i )
  {
    v2 = (float *)&v7[i];
    v3 = strtol(nptr, &nptr, 10);
    *v2 = v3;
    printf((const char *)&v7[i]);
    v7[i] = 0;
  }
  return fflush(stdout);
}

int main() {
  print_flag(190);
  return 0;
}