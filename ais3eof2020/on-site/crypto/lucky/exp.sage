import os
os.environ['PWNLIB_NOTERM'] = '1'

from pwn import *
context.log_level = 'debug'

Z2 = GF(2)
Z32 = GF(2^32)

self_f = 1812433253
(self_w, self_n, self_m, self_r) = (32, 624, 397, 31)
self_a = 0x9908b0df
(self_u, self_d) = (11, 0xffffffff)
(self_s, self_b) = (7, 0x9d2c5680)
(self_t, self_c) = (15, 0xefc60000)
self_l = 18
self_lower_mask = (1 << self_r) - 1
self_upper_mask = (1 << self_r)

def rand(x):
    y = x
    y = y ^^ ((y >> self_u) & self_d)
    y = y ^^ ((y << self_s) & self_b)
    y = y ^^ ((y << self_t) & self_c)
    y = y ^^ (y >> self_l)
    return y & 0xffffffff

def unrand(y):
    RR = rand_mat_repr().inverse()
    y = Z32.fetch_int(y)
    x = Z32(RR * vector(y))
    return x.integer_representation()

def rand_mat_repr():
    RR = matrix(Z2, 32, 32)
    g = Z32.gen()

    for i in range(32):
        y = (g^i).integer_representation()
        y = y ^^ ((y >> self_u) & self_d)
        y = y ^^ ((y << self_s) & self_b)
        y = y ^^ ((y << self_t) & self_c)
        y = y ^^ (y >> self_l)

        RR[:,i] = vector(Z32.fetch_int(y))
    return RR

def twist(state0, state1, state397):
    x = state0 & self_upper_mask
    x += state1 & self_lower_mask
    xA = x >> 1
    if x % 2 != 0:
        xA ^^= self_a
    state0 = state397 ^^ xA
    return state0

r = remote('eof.ais3.org', 39091, timeout=1)
#  r = remote('localhost', 1234, timeout=1)

r.sendlineafter('> ', '1')
r.sendlineafter('> ', '8787878')
r.recvuntil('Lucky Number: ')
state1 = unrand(int(r.recvline()))

r.sendlineafter('> ', '395')
r.sendlineafter('> ', '8787878')
r.recvuntil('Lucky Number: ')
state397 = unrand(int(r.recvline()))

state0 = twist(0, state1, state397) # guess state0 = 0, 1/2 possibility of correctness
print('state[0,1,397] =',state0,state1,state397)

r.sendlineafter('> ', '226')
r.sendlineafter('> ', str(rand(state0)))

print(r.recv(100000))
print(r.recv(100000))
