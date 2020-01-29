from pwn import *
from Crypto.Util.number import *
import subprocess

context.log_level = 'debug'

r = remote('localhost', 3333)

r.sendlineafter('> ', '1')
n = int(r.recvline()[4:])
e = int(r.recvline()[4:])

r.sendlineafter('> ', '2')
c1 = bytes.fromhex(r.recvline().strip()[7:].decode())
r.sendlineafter('> ', '2')
c2 = bytes.fromhex(r.recvline().strip()[7:].decode())
r.sendlineafter('> ', '2')
c3 = bytes.fromhex(r.recvline().strip()[7:].decode())

proc = subprocess.Popen([
    '/opt/SageMath/sage',
    'calc.sage',
    c1.hex(),
    c2.hex(),
    c3.hex(),
    str(e),
    str(n)])
proc.wait()
