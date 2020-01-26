import datetime


## travel to 1985
fake_timestamp = hex(int(datetime.datetime(year=1985, month=1, day=1).timestamp()))
print(fake_timestamp)
# '0x1c36da00'


## decompiled:
    # gets(&password);
    # i = 0;
    # while (sVar3 = strlen(&password), i < sVar3) {
    #   (&password)[i] = (&password)[i] | 0x20;
    #   i = i + 1;
    # }
    # printf("[!] reading ... the.... passw0r..d.....\n");
    # j = 0;
    # while (j < 19) {
    #   (&password)[j] ^= peb->BeingDebugged + ((char)year + 63) * 2 + 127;
    #   if ((&password)[j] != (&gblPass)[j]) {
    #     puts("[!] oops... time machine g0t some trouble in the 0ld tim3... ");
    #     break;
    #   }
    #   j = j + 1;
    # }
    # k = 0;
    # while (k < 19) {
    #   (&password)[k] = (&password)[k] ^ (&gblXorMask)[k];
    #   k = k + 1;
    # }



gblXorMask = bytes.fromhex('5b 5f 51 5f 2a 1c 0a 43 33 02 54 4d 11 02 09 2c 70 71 70')
gblPass = bytearray.fromhex('1d 13 10 18 51 4c 4f 1c 12 51 0b 08 50 51 50 51 50 51 50')
password = bytearray(19)

for i in range(19):
    password[i] = gblPass[i] ^ ( ( 0 + (1985 + 63) * 2 + 127 ) & 0xff )

print(password)
# blog.30cm.tw/./././


for i in range(19):
    gblPass[i] ^= gblXorMask[i]

print(gblPass)
# FLAG{PE_!S_EASY}   .
# without dot at the end