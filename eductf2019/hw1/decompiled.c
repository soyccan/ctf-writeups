
int main(int argc,char **argv,char **envp)

{
  HMODULE pHVar1;
  int *piVar2;
  size_t sVar3;
  int in_FS_OFFSET;
  undefined4 local_3c;
  HMODULE moduleHandle;
  int k;
  int j;
  uint i;
  int iVar1;
  
  FUN_004019e0();
  pHVar1 = GetModuleHandleA((LPCSTR)0x0);
  piVar2 = (int *)((int)&pHVar1->unused + (int)pHVar1[0xf]);
  iVar1 = *(int *)(in_FS_OFFSET + 0x30);
  if ((*(short *)&pHVar1->unused == 0x5a4d) && (*piVar2 == 17744)) {
    printf(
          " --------------------------- \n | B@ck t0 7he Fu7ur3...  \n |en.wikipedia.org/wiki/Back_to_the_Future\n  --------------------------- \n"
          );
    year = getYear(ImageNTHeader->FileHeader.TimeDateStamp);
    printf("[+] It\'s a time machine built in 1985, \n\tand you\'re in %i year now.\n");
    if (year != 1985) {
      puts("[!] WARNING: \n\tit might be some trouble if you\'re not in 1985 year.");
    }
    printf("[!] Time Machine Guarder: %s\n");
    printf("[+] input password to launch time machine: ");
    gets(&password);
    i = 0;
    while (sVar3 = strlen(&password), i < sVar3) {
      (&password)[i] = (&password)[i] | 0x20;
      i = i + 1;
    }
    printf("[!] reading ... the.... passw0r..d.....\n");
    j = 0;
    while (j < 19) {
      (&password)[j] = (&password)[j] ^ *(char *)(iVar1 + 2) + ((char)year + '?') * 2 + 127U;
      if ((&password)[j] != (&gblPass)[j]) {
        puts("[!] oops... time machine g0t some trouble in the 0ld tim3... ");
        break;
      }
      j = j + 1;
    }
    k = 0;
    while (k < 19) {
      (&password)[k] = (&password)[k] ^ (&gblXorMask)[k];
      k = k + 1;
    }
    printf("[+] a flag found by time machine at %i:\n\t%s\n");
  }
  else {
    puts("time machine broken, oohoho. please don\'t patch me ;)");
  }
  return 0;
}

