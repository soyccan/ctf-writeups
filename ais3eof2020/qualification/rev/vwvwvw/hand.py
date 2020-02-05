arr = bytes.fromhex('0a  0b  09  0a  07  08  06  07  04  05  03  04  01  02  00  01  0a  0b  09  0a  07  08  06  07  04  05  03  04  01  02  00  01')
secret = bytes.fromhex('69 7a 12 2d 34 02 28 1a 6a 6e 0e 01 25 13 05 31 6b 57 19 21 26 11 16 32 69 6e 2c 26 36 3f 30 20 00')
path = b'/home/terrynini38514/Desktop'
flag = bytearray(24)

print(len(secret))

for i in range(24):
    flag[i] = secret[i%len(secret)] ^ path[i%len(secret)]
print(flag)
