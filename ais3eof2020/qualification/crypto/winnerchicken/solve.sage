import socket

class remote:
    def __init__(self, host, port):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((host, port))
        self.buffer = b''
    def recvuntil(self, text):
        text = self._convert_to_bytes(text)
        while text not in self.buffer:
            self.buffer += self.s.recv(1024)
        index = self.buffer.find(text) + len(text)
        result, self.buffer = self.buffer[:index], self.buffer[index:]
        return result
    def recvline(self):
        return self.recvuntil(b'\n')
    def recvlines(self, n):
        lines = []
        for _ in range(n):
            lines.append(self.recvline())
        return lines
    def _convert_to_bytes(self, text):
        if type(text) is not bytes:
            text = str(text)
        if type(text) is str:
            text = text.encode()
        return text
    def send(self, text):
        text = self._convert_to_bytes(text)
        self.s.sendall(text)
    def sendline(self, text):
        text = self._convert_to_bytes(text)
        self.send(text + b'\n')
    def sendafter(self, prefix, text):
        self.recvuntil(prefix)
        self.send(text)
    def sendlineafter(self, prefix, text):
        self.recvuntil(prefix)
        self.sendline(text)


F.<x> = GF(2^64)

r = remote('localhost', 1111)

def matrix_repr_step():
    """ matrix representation of step() """
    # step_64x64 =
    #  ( poly\      | I_63x63 )
    #  (     _64x1  | O_1x63  )
    poly = 0xaa0d3a677e1be0bf
    poly = vector(F.fetch_int(poly))
    step = []
    for i in range(64):
        row = [ poly[i] ] + [0] * 63
        if i < 63:
            row[i+1] = 1
        step.append(row)
    step = Matrix(GF(2), step)
    return step

def get_result():
    """ guess 64 times and get result """
    result = []
    money = 1.1
    for _ in range(64):
        r.sendlineafter('> ', '0')
        new_money = float(r.recvline())
        if new_money > money:
            result.append(0)
        else:
            result.append(1)
        money = new_money
    return result

def test_get_result():
    """ local version of get_result(), for testing """
    result = []
    for _ in range(64):
        result.append(random())
    return result

def recover_init_state():
    # solve linear equation system:
    # lsb*step^42        == guess[0]
    # lsb*step^(43+42)   == guess[1]
    # lsb*step^(2*43+42) == guess[2]
    # ... (until guess[63])
    step = matrix_repr_step()
    base = step^42
    mul = step^43
    lsb = vector(F([1] + [0] * 63)).row()
    mat = []
    guess = get_result()
#      guess = test_get_result()
    for _ in guess:
        mat.append((lsb * base).list())
        base *= mul

    guess = vector(F(guess))
    mat = Matrix(GF(2), mat)
    init_state = mat.solve_right(guess) # solve: mat*x = guess

    return F(init_state).integer_representation()

def test_step():
    global state
    print('state0=',state)

    S = matrix_repr_step()
    ss = vector(F.fetch_int(state)).column()
    state1 = F((S * ss).column(0)).integer_representation()
    print('state1=', state1)

    step()
    print('state1=',state)
    assert state == state1

    print()


# from task.py:
def step():
    global state
    poly = 0xaa0d3a677e1be0bf
    out = state & 1
    state >>= 1
    if out:
        state ^^= poly
    return out

def random():
    for _ in range(42):
        step()
    return step()
# end task.py


state = recover_init_state()

for _ in range(64):
    random()
for _ in range(90):
    a = random()
    r.sendlineafter('> ', str(a))
    print(r.recvline())

