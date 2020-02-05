from pwn import *

r = remote('localhost', 1111)
r.interactive()
