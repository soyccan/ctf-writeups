int __cdecl main(int argc, const char **argv, const char **envp)
{
  __int64 v3; // r12
  unsigned int v4; // ebx
  __int64 v5; // rax
  int result; // eax
  int v7; // eax
  void *v8; // rax
  int len; // [rsp+8h] [rbp-58h]
  int x; // [rsp+Ch] [rbp-54h]
  void *ptr; // [rsp+10h] [rbp-50h]
  FILE *stream; // [rsp+18h] [rbp-48h]
  char str[8]; // [rsp+20h] [rbp-40h]
  __int64 v14; // [rsp+28h] [rbp-38h]
  __int64 v15; // [rsp+30h] [rbp-30h]
  int v16; // [rsp+38h] [rbp-28h]
  __int16 v17; // [rsp+3Ch] [rbp-24h]
  unsigned __int64 v18; // [rsp+48h] [rbp-18h]

  v18 = __readfsqword(0x28u);
  *(_QWORD *)str = 0LL;
  v14 = 0LL;
  v15 = 0LL;
  v16 = 0;
  v17 = 0;
  puts("-----");
  puts("I encrypted a image in this binary,");
  puts("you have to answer my questions to decrypt it,");
  puts("cuz only my friends can view this secret image");
  puts("-----");
  puts("Who am I? (lowercase)");
  __isoc99_scanf("%4s", str);
  len = strlen(str);
  puts("How old am I?");
  __isoc99_scanf("%2s", &str[len]);
  len = strlen(str);
  puts("What the name of my first pet? (lowercase)");
  __isoc99_scanf("%20s", &str[len]);
  len = strlen(str);
  v3 = hash(&len);
  v4 = len;
  v5 = hash(str);
  printf("%s, %lu, %d, %lu", str, v5, v4, v3);
  if ( hash(&len) == 177593 && hash(str) == 8932587927620123215LL )
  {
    ptr = &binary_flag_jpg_start;
    x = 0;
    stream = fopen("./flaggggg.jpg", "w");
    if ( stream )
    {
      while ( ptr != &binary_flag_jpg_end )
      {
        v7 = x++;
        *(_BYTE *)ptr ^= str[v7 % len];
        v8 = ptr;
        ptr = (char *)ptr + 1;
        fwrite(v8, 1uLL, 1uLL, stream);
      }
      puts("you got my secret photo");
      fclose(stream);
      result = 0;
    }
    else
    {
      puts("write file error !");
      result = -2;
    }
  }
  else
  {
    puts("haker haker go away");
    result = -1;
  }
  return result;
}
