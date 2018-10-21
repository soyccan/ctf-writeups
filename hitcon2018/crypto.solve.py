#!/usr/bin/env python3
from Crypto.Util.number import *
from pwn import *
import math

context(log_level="DEBUG")

R = remote('13.112.92.9', 21701)
# R = process(['/usr/bin/python2', './crypto-33dee9470e5b5639777f7c50e4c650e3.py'])
R.recvuntil('Here is the flag!')
e_flag = R.recvline(keepends=False)
R.recvuntil('cmd: ')

log.info(f'e_flag={e_flag}')

m = 0
A = []
for i in range(10):
    R.sendline('A')
    R.recvuntil(' ') # input:
    R.sendline(hex(m)[2:].rjust(2,'0'))
    A.append(R.recvline())
    log.info(f'A0={A[0]}')
g = bytes_to_long(A[0])
for a in A:
    g = math.gcd(g, bytes_to_long(a))
log.info(f'gcd={g}')
