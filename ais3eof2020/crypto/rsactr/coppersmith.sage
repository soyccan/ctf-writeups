import sys

def calc(c, nonce, n):
    P.<x> = PolynomialRing(Zmod(n))
    flag = b''
    for i in range(0, len(c), 128):
        nonce = (nonce + 2020) % n
        cc = int.from_bytes(c[i:i+128], 'big')
        f = (cc - x) ^ 3 - nonce
        flag += int(f.monic().small_roots()[0]).to_bytes(16, 'big')
    return flag

print(calc(bytes.fromhex(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])))
