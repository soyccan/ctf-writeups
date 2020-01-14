import string
from hashlib import md5

def unshift(arg):
    arg = bytearray(arg, 'ascii')
    for i in range(0, len(arg), 4):
        arg[i : i+4] = arg[i+1], arg[i+3], arg[i], arg[i+2]

    return arg.decode('ascii')

def bruteforce(md5hash):
    CHR = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789{}_ '
    # CHR = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    # CHR = string.printable[:95]
    for i in range(len(CHR)):
        for j in range(len(CHR)):
            for k in range(len(CHR)):
                for l in range(len(CHR)):
                    guess = CHR[i] + CHR[j] + CHR[k] + CHR[l]
                    # print(guess, md5(guess.encode()).hexdigest(), md5hash)
                    if md5(guess.encode()).hexdigest() == md5hash:
                        return guess

# print(md5('AIS3'.encode()).hexdigest())

enc_flag = open('output.txt').read().split('\n')
for blk in enc_flag:
    blk = unshift(blk)
    print(bruteforce(blk))
# AIS3{7here_15_the_k1ll3r_ra661t}
