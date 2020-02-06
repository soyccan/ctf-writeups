import owiener
from pwn import *
from Crypto.Util.number import *

nbit = 384

#  r = remote('ais3eof.zoolab.org', 9000)
#  n = int(r.recvline()[4:])
#  e = int(r.recvline()[4:])

#  d = owiener.attack(e, n)

#  print(d)

def gcd(a, b):
    if b == 0:
        return (a, 0, 1)
    g, x, y = gcd(b, a % b)
    return (g, y - (a // b) * x, x)

def modinv(a, m):
    g, x, y = gcd(m, a)
    if g != 1:
        return -1
    return x % m

def gen():
    x, y, z, u = [getPrime(nbit) for _ in range(4)]
    phi = (x-1)*(y-1)*(z-1)*(u-1)
    n = x*y*z*u
    while True:
        d = getRandomInteger(192)
        e = modinv(d, phi)
        if e == -1:
            continue
        break
    return n, e, d, phi, [x, y, z, u]

n, e, d, phi, xs = gen()

print('n='+str(n))
print('e='+str(e))
print('d='+str(d))
print('phi='+str(phi))
assert e*d%phi == 1
