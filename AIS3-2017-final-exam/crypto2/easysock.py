import io

class EasySocket(object):
    def __init__(self, suck_socket):
        self.sock = suck_socket

    def recv(self, *args, **kwargs):
        return self.sock.recv(*args, **kwargs)
    def send(self, *args, **kwargs):
        return self.sock.send(*args, **kwargs)
    def sendall(self, *args, **kwargs):
        return self.sock.sendall(*args, **kwargs)

    def write(self, x):
        if type(x) is str:
            x = x.encode()
        self.sock.sendall(x)

    def writeline(self, x):
        self.write(x + '\n')

    def read(self, l=4096):
        return self.sock.recv(l)

    def readline(self):
        buff = io.BytesIO()
        while True:
            x = self.sock.recv(1)
            if not x:
                raise EOFError()
            buff.write(x)
            if x == b'\n':
                break
        return buff.getvalue().decode()

class NumberSocket(object):
    def __init__(self, raw_socket):
        self.sock = EasySocket(raw_socket)
    def write(self, v):
        self.sock.write('%x\n' % v)
    def read(self):
        return int(self.sock.readline(), 16)

