// gcc callme64.c -o callmegg -lcrypto

#include <stdio.h>
#include <stdint.h>
#include <string.h>

unsigned char byte_4012B7[8] = {
    0xDB, 0x7E, 0xE8, 0xB3, 0x7A, 0x0A, 0x1F, 0x51
};
unsigned char encrypted[64] = {
    0xDB, 0x7E, 0xE8, 0xB3, 0x7A, 0x0A, 0x1F, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x7A, 0x50, 0x7C, 0x6F, 0xB7, 0x92, 0x2F, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x55, 0x28, 0x7E, 0x34, 0x87, 0x4B, 0x83, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
int table[10][10] =
{
  { 555, 372, 341, 923, 442, 982, 817, 950, 153, 526 },
  { 909, 135, 136, 994, 428, 690, 534, 915, 145, 744 },
  { 972, 735, 646, 181, 833, 712, 374, 448, 223, 655 },
  { 31, 470, 321, 275, 233, 266, 375, 806, 272, 87 },
  { 123, 609, 892, 892, 692, 130, 269, 112, 689, 869 },
  { 188, 23, 795, 43, 749, 578, 926, 80, 626, 351 },
  { 343, 326, 104, 829, 389, 768, 679, 625, 250, 20 },
  { 188, 23, 795, 43, 749, 578, 926, 80, 626, 351 },
  { 111, 837, 824, 124, 755, 486, 308, 566, 875, 497 },
  { 751, 572, 732, 320, 631, 254, 313, 517, 45, 470 }
}; // idb
char data[2048]; // idb
char input[256]; // idb

//----- (0000000000400A36) ----------------------------------------------------
void output(int x)
{
  int i; // [sp+1Ch] [bp-4h]@1

  for ( i = 0; i <= 7; ++i )
    putchar(*(&encrypted[16 * (x - 1)] + i));
  fflush(stdout);
}

//----- (0000000000400A8E) ----------------------------------------------------
void decrypt(int val, const char *str, int x)
{
  size_t v3; // rax@1
  size_t v4; // rax@29
  uint64_t v5; // rax@32
  const char *stra; // [sp+0h] [bp-190h]@1
  int xa; // [sp+8h] [bp-188h]@1
  int i; // [sp+10h] [bp-180h]@1
  int ia; // [sp+10h] [bp-180h]@4
  int ib; // [sp+10h] [bp-180h]@10
  int ic; // [sp+10h] [bp-180h]@13
  int id; // [sp+10h] [bp-180h]@29
  int j; // [sp+14h] [bp-17Ch]@5
  int ja; // [sp+14h] [bp-17Ch]@14
  int jb; // [sp+14h] [bp-17Ch]@21
  int a; // [sp+18h] [bp-178h]@14
  int m; // [sp+1Ch] [bp-174h]@1
  int ma; // [sp+1Ch] [bp-174h]@14
  uint64_t flag[6]; // [sp+20h] [bp-170h]@1
  int dis[10]; // [sp+50h] [bp-140h]@11
  char tmp[256]; // [sp+80h] [bp-110h]@1

  stra = str;
  xa = x;
  flag[0] = 0LL;
  flag[1] = 0LL;
  flag[2] = 0LL;
  flag[3] = 0LL;
  flag[4] = 0LL;
  memset(data, 0, 0x800uLL);
  v3 = strlen(str);
  SHA256(str, v3, tmp);
  m = 0;
  for ( i = 0; i <= 7; ++i )
    m += (uint8_t)tmp[i];
  for ( ia = 0; ia <= 9; ++ia )
  {
    for ( j = 0; j <= 9; ++j )
      table[0][j + 10LL * ia] += m;
  }
  for ( ib = 0; ib <= 9; ++ib )
    dis[ib] = 0x3FFFFFFF;
  dis[val] = 0;
  for ( ic = 0; ic <= 9; ++ic )
  {
    a = -1;
    ma = 0x3FFFFFFF;
    for ( ja = 0; ja <= 9; ++ja )
    {
      if ( !*((uint32_t *)flag + ja) && dis[ja] < ma )
      {
        a = ja;
        ma = dis[ja];
        sprintf(tmp, "%d%d", (unsigned int)ja, (unsigned int)ma);
        strcat(data, tmp);
      }
    }
    if ( a == -1 )
      break;
    *((uint32_t *)flag + a) = 1;
    for ( jb = 0; jb <= 9; ++jb )
    {
      if ( !*((uint32_t *)flag + jb) && dis[a] + table[0][jb + 10LL * a] < dis[jb] )
        dis[jb] = dis[a] + table[0][jb + 10LL * a];
    }
  }
  sprintf(tmp, "%d", (unsigned int)dis[9]);
  strcat(data, tmp);
  v4 = strlen(data);
  SHA256(data, v4, tmp);
  for ( id = 0; id <= 7; ++id )
    *(&encrypted[16 * (xa - 1)] + id) ^= tmp[id];
}
//----- (0000000000400F02) ----------------------------------------------------
void callme1()
{
  strcpy(input, "HELLOWORLD");
  decrypt(1, input, 1);
  output(1);
}

void callme3()
{
  size_t v0; // rax@7
  char buf[256]; // [sp+0h] [bp-110h]@1

  memset(buf, 0, sizeof(buf));
  strcpy(input, "LASTONEGOGOG0");
  memcpy(encrypted, byte_4012B7, 8uLL);
  strncpy(buf, input, 0xDuLL);
  strncpy(buf, encrypted, 8uLL);
  v0 = strlen(buf);
  SHA256(buf, v0, buf);
  decrypt(4, buf, 3);
  output(3);
}

//----- (00000000004010CB) ----------------------------------------------------
void callme2()
{
  size_t v0; // rax@4
  const char s[15]; // [sp+0h] [bp-60h]@1
  char buf[64]; // [sp+10h] [bp-50h]@1

  memset(buf, 0, sizeof(buf));
  strcpy((char *)s, "YOUAREANEXPERT");
  strcpy(input, s);
  v0 = strlen(s);
  SHA256(input, v0, buf);
  decrypt(2, buf, 2);
  output(2);
}
// 4008C0: using guessed type int __fastcall SHA256(_QWORD, _QWORD, _QWORD);

//----- (000000000040119A) ----------------------------------------------------
int main(int argc, const char **argv, const char **envp)
{
  callme1();
  callme2();
  callme3();
}