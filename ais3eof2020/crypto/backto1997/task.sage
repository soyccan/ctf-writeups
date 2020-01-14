import os

m = 128
F = GF(2^m)


def Enc(x, y, ks):
    for k in ks:
        x, y = y, x + (1 / (y + k))
    return x, y


def F_random_element():
    return F.fetch_int(int(os.urandom(m//8).encode('hex'), 16))


ks = [F_random_element() for _ in range(6)]

with open('../flag.txt') as f:
    flag = f.read().strip()
    assert len(flag) == m * 2 // 8 # = 32
    x0 = F.fetch_int(int(flag[:m//8].encode('hex'), 16))
    y0 = F.fetch_int(int(flag[m//8:].encode('hex'), 16))
    x, y = Enc(x0, y0, ks)
    print(''.join('%42d' % e.integer_representation() for e in [x, y]))

for _ in range(42 * 10):
    x0, y0 = F_random_element(), F_random_element()
    x, y = Enc(x0, y0, ks)
    print(''.join('%42d' % e.integer_representation() for e in [x0, y0, x, y]))
