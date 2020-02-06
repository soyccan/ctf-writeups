from pwn import *
import sys
import owiener
import subprocess

def iter_factor(fact, res, n, idx):
    if idx >= len(fact):
        res.append(n)
        return

    base, exp = fact[idx]
    for e in range(0, exp+1):
        iter_factor(fact, res, n, idx+1)
        n *= base

r = remote('ais3eof.zoolab.org', 9000, timeout=1)

n = int(r.recvline()[4:])
e = int(r.recvline()[4:])
phi = owiener.attack(e, n)
success('phi='+str(phi))
