import re
import requests
from Crypto.Cipher import DES, AES
import sys

def rsplit(instr, block_size=8):
    instr = list(instr)
    outstr = []
    while instr:
        b = []
        for i in range(block_size):
            if not instr:
                break
            b = [instr.pop()] + b
        outstr = [''.join(b)] + outstr
    return outstr


html = requests.get('http://13.115.255.46').text
# print(html)

for match in re.finditer(r'<a href="\?s=([0-9a-f]+)">([^<]+)</a>', html):
    ciphertext = match.group(1)
    title = match.group(2)
    r = requests.get('http://13.115.255.46/?s=' + ciphertext, allow_redirects=False)
    url = r.headers.get('Location')

    L1 = rsplit(ciphertext, 16)
    L2 = rsplit(title, 8)
    sys.stdout.write(f'{L1}\n{L2}\n{url}\n\n')

# for i in range(26**8):
#     key = ''
#     while i > 0:
#         key += chr(ord('a') + i % 26)
#         i //= 26
#     while len(key) < 8:
#         key += 'a'
#     print(f'key={key}')
#     cipher = DES.DESCipher(key, DES.MODE_ECB)
#     continue
#     for line in open('ohmyraddit.log'):
#         ciphertext, title, url = line.split('\t')
#         url = url.rstrip('/')
#         plaintext = cipher.decrypt(ciphertext)

#         print(len(ciphertext)/8,len(title),len(url))
#         print(ciphertext)
#         print(plaintext)
#         print(title)
#         print(url)
#         print()


