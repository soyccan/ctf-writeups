class RNG:
    def __init__(self):
        self.f = 1812433253
        (self.w, self.n, self.m, self.r) = (32, 624, 397, 31)
        self.a = 0x9908b0df
        (self.u, self.d) = (11, 0xffffffff)
        (self.s, self.b) = (7, 0x9d2c5680)
        (self.t, self.c) = (15, 0xefc60000)
        self.l = 18
        self.lower_mask = (1 << self.r) - 1
        self.upper_mask = (1 << self.r)

        self.index = self.n + 1
        self.state = [0]*self.n
        self.seed = None

    def srand(self, seed):
        self.seed = seed
        self.state[0] = seed & 0xffffffff

        for i in range(1, self.n):
            self.state[i] = (self.f * (self.state[i-1] ^^ (self.state[i-1] >> (self.w-2))) + i) & 0xffffffff

    def rand(self):
        if self.seed == None:
            self.srand(random.randrange(1, 0xffffffff))
        if self.index >= self.n:
            self.twist()

        y = self.state[self.index]
        y = y ^ ((y >> self.u) & self.d)
        y = y ^ ((y << self.s) & self.b)
        y = y ^ ((y << self.t) & self.c)
        y = y ^ (y >> self.l)

        self.index += 1

        return y & 0xffffffff

    def twist(self):
        for i in range(self.n):
            x = self.state[i] & self.upper_mask
            x += self.state[(i+1) % self.n] & self.lower_mask
            xA = x >> 1
            if x % 2 != 0:
                xA ^^= self.a
            self.state[i] = self.state[(i+self.m) % self.n] ^^ xA
        self.index = 0


Z2 = GF(2)
Z2_32 = GF(2^32)
VS32 = GF(2)^32

self_f = 1812433253
(self_w, self_n, self_m, self_r) = (32, 624, 397, 31)
self_a = 0x9908b0df
(self_u, self_d) = (11, 0xffffffff)
(self_s, self_b) = (7, 0x9d2c5680)
(self_t, self_c) = (15, 0xefc60000)
self_l = 18
self_lower_mask = (1 << self_r) - 1
self_upper_mask = (1 << self_r)

def my_rand(x):
    y = x
    y = y ^^ ((y >> self_u) & self_d)
    y = y ^^ ((y << self_s) & self_b)
    y = y ^^ ((y << self_t) & self_c)
    y = y ^^ (y >> self_l)
    return y & 0xffffffff

def rand_mat_repr():
    RR = matrix(Z2, 32, 32)
    g = Z2_32.gen()


    for i in range(32):
        y = (g^i).integer_representation()
        y = y ^^ ((y >> self_u) & self_d)
        y = y ^^ ((y << self_s) & self_b)
        y = y ^^ ((y << self_t) & self_c)
        y = y ^^ (y >> self_l)

        RR[:,i] = vector(Z2_32.fetch_int(y))
    return RR


def srand_mat_repr():
    SR = matrix(Z2, 32, 32)
    g = Z2_32.gen()
    for j in range(32):
        sb = (g^j).integer_representation()
        sb = (1812433253 * (sb ^^ (sb >> 30))) & 0xffffffff
        sb = Z2_32.fetch_int(sb)
        SR[:,j] = vector(sb)
    return SR

def test_srand():
    SR = srand_mat_repr()
    print(SR)

    rng = RNG()
    rng.srand(248932)
    for i in range(1, 624):
        print(i)
        assert rng.state[i] == Z2_32(SR*vector(Z2_32.fetch_int(rng.state[i-1]))).integer_representation() + i



def twist_mat_repr():
#      TT = matrix.identity(Z2, 624*32)
#      upper_mask = 1 << 31
#      lower_mask = 1 << 31 - 1

#      for j in range(624*32):
#          print(j)
#          sb = vector(Z2, 624*32) # standard basis
#          sb[j] = 1
#          for i in range(0, 624*32, 32):
#              x = Z2_32(sb[i:i+32]).integer_representation() & upper_mask
#              t = (i+1)%624 * 32
#              x += Z2_32(sb[t:t+32]).integer_representation() & lower_mask
#              xA = x >> 1
#              xA ^^= 0x9908b0df * (x & 1)
#              t = (i+397)%624 * 32
#              sb[i:i+32] = vector(Z2_32.fetch_int(
#                  Z2_32(sb[t:t+32]).integer_representation() ^^ xA))
#          TT[:,j] = sb

#          if j > 64: break

#      import pdb; pdb.set_trace()

    TT = matrix(Z2, 624*32, 624*32)

    va = vector(Z2_32.fetch_int(0x9908b0df))
    xa = matrix(Z2, 32, 64)
    xa[:,31] = xa[:,32] = va
    xa[0:31,33:64] = matrix.identity(Z2, 31)

    for i in range(0, 624*32, 32):
        if i < 623*32:
            TT[i:i+32 , i:i+64] = xa
        else:
            TT[i:i+32 , i:i+32] = xa[:,0:32]
            TT[i:i+32 , 0:32] = xa[:,32:64]

    I32 = matrix.identity(Z2, 32)
    for i in range(624):
        if i < 227:
            t = (i+397)%624 * 32
            TT[i*32:(i+1)*32 , t:t+32] = I32
        else:
            t = (i+397*2)%624 * 32
            TT[i*32:(i+1)*32 , t:t+32] = I32

            t = (i+397)%624 * 32
            TT[i*32:(i+1*32) , t:t+64] = xa

    return TT

def test_twist():
    TT = twist_mat_repr()

    state = [ Z2_32.random_element().integer_representation() for i in range(624) ]
    statev = []
    for s in state:
        statev += vector(Z2_32.fetch_int(s)).list()
    statev = vector(statev)

    resv = TT * statev
    res = []
    for i in range(0, len(resv), 32):
        res.append(Z2_32(resv[i:i+32]).integer_representation())

    rng = RNG()
    rng.state = state
    rng.twist()
    assert rng.state == res

#  test_twist()
#  test_srand()
R = rand_mat_repr()
RR = R.inverse()

y = Z2_32.random_element()
x = Z2_32(RR*vector(y))
assert my_rand(x.integer_representation()) == y.integer_representation()
