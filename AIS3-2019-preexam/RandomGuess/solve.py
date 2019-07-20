from pwn import *
import functools
import math
import gmpy2

def calc(arr):
    '''arr[n+1] = (a * arr[n] + b) mod c'''
    diff = []
    for i in range(len(arr) - 1):
        diff.append(arr[i + 1] - arr[i])

    mul = [] # multiples of c
    for i in range(1, len(diff) - 1):
        mul.append(diff[i+1] * diff[i-1] - diff[i] * diff[i])

    c = mul[0]
    for i in range(1, len(mul)):
        c = math.gcd(c, mul[i])

    a = diff[2] * gmpy2.invert(diff[1], c) % c # this is (a mod n)
    b = (arr[1] - a * arr[0]) % c

    return (a, b, c)


def main():
    '''
    Random number a,b,c :
    Given the set of number Ni+1 = (a * Ni + b) % c
    N = 781047696, 127007304, 265700385, 226577991, 246116750, 817822430, 315085643, 857238524, 552495325, 629072513
    If you can guess the next 100 numbers to me (one by one), and I will give you the flag A___A+
    '''
    context.log_level = 'DEBUG'

    r = remote('pre-exam-chals.ais3.org', 10200)
    r.recvuntil('N = ')
    arr = list(map(int, r.recvline().split(b',')))
    r.recvuntil('A___A+\n')

    a, b, c = calc(arr)
    an = arr[-1]
    for _ in range(100):
        an = (a * an + b) % c
        r.sendline(str(int(an)).encode())
        r.recvline()
    r.recv()

main()
