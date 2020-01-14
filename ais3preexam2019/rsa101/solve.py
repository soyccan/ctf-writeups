from pwn import *
from Crypto.Util.number import *
import math, gmpy2


# context.log_level = 'DEBUG'

r = remote('pre-exam-chals.ais3.org', 10201)
r.recvuntil('Public-key (e,N) : (')
e = int(r.recvuntil(',').strip(b','))
N = int(r.recvuntil(')').strip(b')'))
r.recvuntil('Encrypted Flag : ')
c = int(r.recvuntil('\n').strip(b'\n'))

print(f'e={e}\nN={N}\nc={c}\n')

stride = 1
while 64 * stride < N:
    stride *= 2
stride //= 2

steps = 0
while stride >= 1:
    steps += stride
    r.recvuntil('n = ? \n')
    r.send(str(64 * steps).encode())
    r.recvuntil('(n % phi) % 64 = ')
    y = int(r.recvline())
    # print(f'y={y} stride={stride} steps={steps}')
    if y != 0:
        steps -= stride
        stride //= 2

phi = 64 * steps + (64 - y)
print(f'phi={phi}')

d = gmpy2.invert(e, phi)
m = pow(c, d, N)
print(long_to_bytes(m))
# AIS3{RSA_L0L_01100110011101010110001101101011}
