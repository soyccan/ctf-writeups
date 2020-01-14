from pwn import *
from Crypto.Cipher import ARC4
import json

#  r = remote('harc4.balsnctf.com', 5450)

#  r.recvuntil('> ')
#  r.sendline('_debug')
#  r.recvuntil('> ')
#  db = r.recvuntil('}')
#  db = json.loads(db)
#  pass_enc = db['admin']
pass_enc = '5c49bc8d60140be15cf8feee00e2c0c07ea1d03842fa1ba730b918a83e74ed75'

state = bytes.fromhex(pass_enc)
passwd = []
for i in range(100):
    for key in range(256):
        key = key.to_bytes(1, 'little')
        cipher = ARC4.new(key)
        prev_state = cipher.decrypt(state)
        if prev_state == key[-32:]:
            passwd.append(key[:-32])
            break
print(passwd)

#  r.recvuntil('> ')
#  r.sendline('login')
#  r.recvuntil('Username: ')
#  r.sendline('admin')
#  r.recvuntil('Password: ')
#  r.sendline(passwd)
#  r.interactive()
