class XorKPA:
    """XOR Known Plaintext / Known Key Length Attack"""
    def __init__(self, key_length, cipher_file):
        self.cipher_file = cipher_file
        self.key = bytearray(key_length)
        self.certain_index = set()

    def add_known_plain(self, position, plain):
        self.cipher_file.seek(position)
        cipher = self.cipher_file.read(len(plain))

        for i in range(len(plain)):
            offset = (position + i) % len(self.key)
            if (offset in self.certain_index
                    and self.key[offset] != cipher[i] ^ plain[i]):
                print('fail', i, position, plain)
            else:
                self.certain_index.add(offset)
                self.key[offset] = cipher[i] ^ plain[i]

    def decrypt(self, position, length):
        self.cipher_file.seek(position)
        cipher = self.cipher_file.read(length)
        plain = bytearray(length)

        for i in range(length):
            offset = (position + i) % len(self.key)
            plain[i] = self.key[offset] ^ cipher[i]

        return bytes(plain)



inf = open('0neWay.jpg.enc', 'br')
outf = open('0neWay.jpg', 'bw')

cracker = XorKPA(20, inf)
cracker.add_known_plain(0, bytes.fromhex('ffd8 ffe0'))
cracker.add_known_plain(6, b'JFIF\x00')
cracker.add_known_plain(0x117, b'http://www.')
cracker.add_known_plain(0x72, b'http://ns.adobe.com/')
print(bytes(cracker.key))
print(cracker.certain_index)
outf.seek(0)
outf.write(cracker.decrypt(0, 183369))
# key = nini22ididnothavepet



# jpg_version = ' 01 01'
# jpg_size = '0010'
# jpg_head = bytes.fromhex('ffd8 ffe0' + jpg_size + '4a46 4946 00' +jpg_version+ '00 0001 0001 0000')
# assert len(jpg_head) == 20

# data = inf.read(20)
# key = b''.join((data[i] ^ jpg_head[i]).to_bytes(1,'little') for i in range(20))
# print(key)
# # key = nini~~ididn~~~~~~~~~
# # ~ means unknown

# inf.seek(-2, 2)
# data = inf.read(2)
# print(chr(data[0] ^ 0xff), chr(data[1] ^ 0xd9))
# # 183369 % 20 == 9
# # key[7:8] = di

# i = 0
# inf.seek(0)
# outf.seek(0)
# while True:
#     data = inf.read(20)
#     if len(data) == 0:
#         break
#     outf.write(b''.join((data[i] ^ key[i]).to_bytes(1,'little') for i in range(len(data))))
