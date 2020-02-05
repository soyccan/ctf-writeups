from pwn import *
import subprocess

context.log_level = 'debug'

#  r = remote('rayfish.zoolab.org', 20000)
r = remote('localhost', 1111)

r.sendlineafter('> ', '1')
n = int(r.recvline()[4:])
e = int(r.recvline()[4:])

r.sendlineafter('> ', '3')
r.sendlineafter('plain = ', '00')
nonce = bytes.fromhex(r.recvline()[9:].decode())
nonce = pow(int.from_bytes(nonce, 'big'), 3, n)

r.sendlineafter('> ', '2')
c = bytes.fromhex(r.recvline()[7:].decode())

proc = subprocess.run([
    '/opt/SageMath/sage',
    'coppersmith.sage',
    c.hex(),
    str(nonce),
    str(n)], capture_output=True)
print(proc.stdout.decode())
print(proc.stderr.decode())
