#!/usr/bin/env python3
import requests
import hashlib

comm = b'Snoopy_do_not_like_cats_hahahahaddaa_is_PHD'
p1 = open('shattered-1.pdf', 'rb').read(320) + comm
p2 = open('shattered-2.pdf', 'rb').read(320) + comm

assert hashlib.sha1(p1).digest() == hashlib.sha1(p2).digest()

for x in range(1000000000):
    if hashlib.sha1(p1 + str(x).encode()).hexdigest()[:4] == 'f00d':
        p1 += str(x).encode()
        p2 += str(x).encode()
        break

print('[*] Hash1 = %s' % hashlib.sha1(p1).hexdigest())
print('[*] Hash2 = %s' % hashlib.sha1(p2).hexdigest())

resp = requests.post('https://quiz.ais3.org:32670/', data={
    'username': p1,
    'password': p2
})

t = resp.text
i = t.find('Flag1:')
print(t[i:])
