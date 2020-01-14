from pwn import *
from Crypto.Util.number import *
import gmpy2

#  context.log_level = 'debug'

r = remote('eductf.zoolab.org', 20000)

# info
r.sendlineafter('> ', '1')
r.recvuntil('n = ')
n = int(r.recvline().strip(b'\n'))
r.recvuntil('e = ')
e = int(r.recvline().strip(b'\n'))

# encrypt and get nonce
r.sendlineafter('> ', '3')
r.sendlineafter('plain = ', (b'\0'*(16 * 100)).hex())
r.recvuntil('cipher = ')
cipher = bytes.fromhex(r.recvline().strip(b'\n').decode())
assert len(cipher) % 128 == 0
cipher = [ int.from_bytes(cipher[ i:i+128 ], 'big') for i in range(0, len(cipher), 128) ]
plain = [ pow(x, e, n) for x in cipher ]
#  nonce = pow(bytes_to_long(cipher[:16]), 3, n)
nonce = plain[0]
print(f'nonce={nonce}')
#  print('cipher=', '\n\n'.join(map(str,cipher)))
#  print('plain=', '\n\n'.join(map(str,plain)))

# get flag
r.sendlineafter('> ', '2')
r.recvuntil('flag = ')
flag_cipher = bytes.fromhex(r.recvline().strip(b'\n').decode())
flag_cipher = int.from_bytes(flag_cipher, 'big')





#  f = open('cipher.txt', 'w')
#  f.write(str(nonce) + ',\n')
#  for x in cipher:
#      f.write(str(x) + ',\n')


for i in range(1, len(plain)):
    assert (plain[i] - plain[i-1] - 2020) % n == 0
#      y = cipher[i] - cipher[i-1]
#      nonce = plain[i-1]
#      assert (pow(y,3,n) + 3*nonce*pow(y,2,n) + 3*nonce*nonce*y - 2020) % n == 0

#  y = 2020
#  for i in range(30000):
#      y += n
#      print(f'y={y}')
#      if gmpy2.iroot(y, 3)[1]:
#          print('yes')
#          break
#  t0 = bytes_to_long(cipher[:16])
#  t1 = bytes_to_long(cipher[16:32])
#  d = gmpy2.iroot(pow(4040*nonce, 3, n) + 2020, 3)[0]
#  print(f't0 = {t0}')
#  print(f't1 = {t1}')
#  print(f'd = {d}')
#  assert (t1 - t0 - d) % n == 0

#  for i in range(500):
#      t = gmpy2.iroot(nonce + 2020 + i*n, 3)[0]
#      flag = long_to_bytes(flag_cipher - t)
#      print(flag)


