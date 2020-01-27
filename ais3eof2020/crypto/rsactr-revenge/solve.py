from pwn import *
from Crypto.Util.number import *
import subprocess

context.log_level = 'debug'

r = remote('localhost', 1111)

r.sendlineafter('> ', '1')
n = int(r.recvline()[4:])
e = int(r.recvline()[4:])

r.sendlineafter('> ', '2')
c1 = bytes.fromhex(r.recvline()[7:-1].decode())
r.sendlineafter('> ', '2')
c2 = bytes.fromhex(r.recvline()[7:-1].decode())
r.sendlineafter('> ', '2')
c3 = bytes.fromhex(r.recvline()[7:-1].decode())

proc = subprocess.run([
    '/opt/SageMath/sage',
    'coppersmith.sage',
    c1.hex(),
    c2.hex(),
    c3.hex(),
    str(e),
    str(n)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
print(proc.stdout.decode())
print(proc.stderr.decode())
