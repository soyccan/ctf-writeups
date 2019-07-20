from sifur_decryptor import sifur_dec
from Crypto.Util import number

class Sifur(object):
    def __init__(self, p=None, g=None, x=None, s=None):
        self.bits = 512
        self.p = p or number.getPrime(self.bits + 1)
        self.g = g or number.getPrime(512)
        self.x = x or number.getPrime(512)
        self.A = pow(self.g, self.x, self.p)
        self.s = s or None
        self.S = None
        self.pub = (self.A, self.g, self.p)

    def second(self, k, l=None):
        assert 0 < k < self.p
        if l is not None:
            assert 0 < l < self.p
        self.s = pow(k, self.x, self.p)
        if l is not None:
            nonce = self.decrypt(l, as_int=True)
            return self.encrypt(nonce * nonce % self.p)

    def encrypt(self, x):
        """
        x can be (str, bytes, int)
        return data will be int
        """
        if type(x) is str:
            x = x.encode()
        if type(x) is bytes:
            x = number.bytes_to_long(x)

        assert 0 < x < self.p
        return (x * self.s) % self.p

    def decrypt(self, x, as_int=False):
        """
        x can be (str, bytes, int)
        return data will be bytes
        """
        if type(x) is str:
            x = x.encode()
        if type(x) is bytes:
            x = number.bytes_to_long(x)

        assert 0 < x < self.p
        if as_int:
            return sifur_dec(self, x)
        else:
            return number.long_to_bytes(sifur_dec(self, x))

class SifurSocket(object):
    def __init__(self, num_sock, sifur):
        """
        num_sock  a NumberSock object
        sifur     a Sifur object
        """
        self.sock = num_sock
        self.sifur = sifur
        self.buffer = b''

    def sendall(self, x):
        self.send(x)

    def send(self, x):
        """
        split x into multiple packets, encrypt them and send
        each packet has (sifur.bits / 8) bytes, first byte is payload length
        """
        bs = self.sifur.bits // 8 - 1
        for i in range(0, len(x), bs):
            data = bytearray(b'\0' + x[i:i+bs])
            data[0] = len(data) - 1
            payload = self.sifur.encrypt(bytes(data))
            self.sock.write(payload)

    def recv(self, l=4096):
        while True:
            if len(self.buffer) >= l:
                r = self.buffer[:l]
                self.buffer = self.buffer[l:]
                return r

            c = self.sock.read()
            x = self.sifur.decrypt(c)
            bs = x[0]
            self.buffer += x[1:].ljust(bs, b'\0')
