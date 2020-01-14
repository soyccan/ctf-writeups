
from pwn import *
from pwnlib.util.iters import mbruteforce
import requests
import string

with open('shattered-1.pdf', 'rb') as f1:
	d1 = f1.read()

with open('shattered-2.pdf', 'rb') as f2:
	d2 = f2.read()

d1 += b'Snoopy_do_not_like_cats_hahahahaddaa_is_PHD'
d2 += b'Snoopy_do_not_like_cats_hahahahaddaa_is_PHD'

s = mbruteforce(lambda x: sha1sumhex(d1 + x.encode()).startswith('f00d'), string.printable, 10)

d1 = d1 + s.encode()
d2 = d2 + s.encode()

payload = {'username' : d1, 'password' : d2}
r = requests.post('https://quiz.ais3.org:32670/', data=payload)

print(r.text)

