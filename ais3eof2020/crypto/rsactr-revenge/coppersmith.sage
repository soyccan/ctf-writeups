import sys

def gcd(f1, f2):
    print('f1',f1)
    print('f2',f2)
    print()
    return f1.monic() if f2 == 0 else gcd(f2, f1 % f2)

c1 = bytes.fromhex(sys.argv[1])
c2 = bytes.fromhex(sys.argv[2])
c3 = bytes.fromhex(sys.argv[3])
e = int(sys.argv[4])
n = int(sys.argv[5])

P.<x> = PolynomialRing(Zmod(n))

flag = b''
for i in range(0, len(c1), 128):
    cc1 = int.from_bytes(c1[i:i+128], 'big')
    cc2 = int.from_bytes(c2[i:i+128], 'big')
    cc3 = int.from_bytes(c3[i:i+128], 'big')

    f1 = (cc1 - x)^e - (cc2 - x)^e + 2020 * (len(c1) // 128)
    f2 = (cc2 - x)^e - (cc3 - x)^e + 2020 * (len(c1) // 128)

    xx = -gcd(f1, f2).monic().coefficients[0]
    flag += int(xx).to_bytes(16, 'little')
print(flag)

#
# (c1 - flag) ^ e = nonce
# (c2 - flag) ^ e = nonce + 2020
# (c3 - flag) ^ e = nonce + 4040
#
# becomes:
# (c2 - flag) ^ e - (c1 - flag) ^ e = 2020
# (c3 - flag) ^ e - (c2 - flag) ^ e = 2020

