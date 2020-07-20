import socket

class remote:
    def __init__(self, host, port, timeout=20):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.settimeout(timeout)
        self.s.connect((host, port))
        logging.info('connected')
        self.buffer = b''

    def interactive(self):
        os.dup2(self.s.fileno(), 0)
        os.dup2(self.s.fileno(), 1)

    def recvuntil(self, text):
        text = self._convert_to_bytes(text)
        while text not in self.buffer:
            self.buffer += self.s.recv(1024)
        index = self.buffer.find(text) + len(text)
        result, self.buffer = self.buffer[:index], self.buffer[index:]
        logging.debug('received:')
        logging.debug(result)
        return result

    def recvline(self):
        return self.recvuntil(b'\n')

    def recv(self, n=10000):
        result = self.s.recv(n)
        logging.debug('received:')
        logging.debug(result)
        return result

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
        logging.debug('sent:')
        logging.debug(text)

    def sendline(self, text):
        text = self._convert_to_bytes(text)
        self.send(text + b'\n')

    def sendafter(self, prefix, text):
        self.recvuntil(prefix)
        self.send(text)

    def sendlineafter(self, prefix, text):
        self.recvuntil(prefix)
        self.sendline(text)
