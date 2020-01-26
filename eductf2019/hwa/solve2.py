''' RSA Chosen Ciphertext Oracle Attack '''
#  from ptrlib import Socket
from pwn import *
from fractions import Fraction

r = remote('edu-ctf.csie.org', 10192)

r.sendlineafter('> ', '1')

r.recvuntil('c = ')
c = int(r.recvline().strip(b'\n'))

r.recvuntil('e = ')
e = int(r.recvline().strip(b'\n'))

r.recvuntil('n = ')
n = int(r.recvline().strip(b'\n'))


# start from 190 since plaintext is small
lower_bound = 0
upper_bound = Fraction(n, pow(16, 190-1))
chosen_cipher = pow(16, 190*e, n) * c % n # = 16^ie * c % n
k = pow(16, e, n)
for i in range(190, 190+261):
    print(f'l={float(lower_bound)}\n u={float(upper_bound)}\n k={k}\n i={i}\n cc={chosen_cipher}\n')

    r.sendlineafter('> ', '2')
    r.sendline(str(chosen_cipher))
    r.recvuntil('m = ')
    oracle = int(r.recvline())
    # oracle = (chosen_cipher)^d % n % 16

    interval = (upper_bound - lower_bound) / 16
    if interval < 0.1: break
    print(f'int={interval}')

    for j in range(16):
        # 16-ary search
        print(f'o={oracle} -{j}%n%16={-j*n%16}')

        if oracle == -j * n % 16:
            lower_bound += interval * j
            upper_bound = lower_bound + interval
            print(f'l={lower_bound}\n u={upper_bound}\n o={oracle}\n j={j}\n')

    chosen_cipher = chosen_cipher * k % n

flag = int(upper_bound)
flag = flag.to_bytes(flag.bit_length() // 8 + 1, 'big')
print(flag.strip(b'\0'))
# FLAG{Youg0tTH3Fl4GIHavesPoKEN}

