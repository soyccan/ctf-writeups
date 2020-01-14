from pwn import *

context.log_level = 'DEBUG'
r = remote('pre-exam-chals.ais3.org', 10202)
r.recvuntil('move: ')
for i in range(16):
    for _ in range(15):
        if i % 2 == 0:
            r.sendline('right')
        else:
            r.sendline('left')
        r.recvline()
    r.sendline('down')
    r.recvuntil('move: ')
