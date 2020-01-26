# 404018, first 32 bytes of A
A1 = bytearray.fromhex('0f 09 31 0c f8 14 ed 36 fa ee e2 ed 36 1e 36 0c 35 3c 36 3c ed 30 36 ef 31 e8 ee ef e9 e2 ec c6')

# 404038, last 32 bytes of A
A2 = bytes.fromhex('00 00 d1 00 00 e6 43 ba 28 34 18 43 ba 04 ec ee fb b4 ec e6 48 bd ae 07 c9 fc fe 46 f8 40 36 00')

# 404058, shellcode
B = bytearray.fromhex('45 7b dc 41 b7 35 ec f0 f0 f0 f0 db f9 7b 35 ec 73 b0 f1 79 35 ec 7b 3d fc f3 3d ec ff ae 01 75 c2 64 15 7b 35 f8 f3 35 ec ff ae f8 73 b1 13 73 e1 56 ff ae c1 7b 35 fc f3 35 ec ff ae f8 2b c1 64 f4 23 b0 db f7 db b5 a8 f1 f0 f0 f0 7b d5 4d b3')


# decrypt_secret()
# by `push ebp` is 0x55 && B[i] == 0x45
# => seed = 16
seed = 16
for i in range(len(A1)):
    A1[i] = (A1[i] + A2[i]) & 0xff

for i in range(len(B)):
    B[i] = (B[i] + seed) & 0xff


# check_flag()
# an interesting truth:
# if use A1 to caluculate flag (without adding A2) will get:
#   FL4G{Oh-yeah-U-G07-7h3-f4keflag}
# but the real flag is:
#   FLAG{y3s!!y3s!!y3s!!0h_my_g0d!!}
flag = bytearray()
for i in range(len(A1)):
    if A1[i] == 0:
        break
    flag.append((A1[i] ^ 0x66) - 0x23);
print(flag)