from pwn import *
import owiener

r = remote('ais3eof.zoolab.org', 9000, timeout=1)

n = int(r.recvline()[4:])
e = int(r.recvline()[4:])
phi = owiener.attack(e, n)
success('phi='+str(phi))
