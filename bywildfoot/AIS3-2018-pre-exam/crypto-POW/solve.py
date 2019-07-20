# -*- coding: utf-8 -*-
#!/usr/bin/env python
#   Version 
#   Author: WildfootW
#   GitHub: github.com/Wildfoot
#   Copyright (C) 2018 WildfootW All rights reserved.
#

from pwn import *
from hashlib import sha256

context.arch = "amd64"
context.os = "linux"
context.endian = "little"
# ["CRITICAL", "DEBUG", "ERROR", "INFO", "NOTSET", "WARN", "WARNING"]
context.log_level = "DEBUG"

host = "104.199.235.135"
port = 20000
while(True):
    r = remote(host, port)

    r.recvline()
    r.recvline()
    condition = r.recvline()[22:28]
    r.recvuntil("x = ")

    def simple_test(stringA):
        #print(".", end = "")
        sha_result = sha256(bytes(stringA, "utf-8")).hexdigest()
        #log.info(stringA + " : " + sha_result)
        if(sha_result[:6] == "000000"):
            log.info(stringA + " : " + sha_result)
            return True
        return False

    char_pool = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    new_str = ""

    for i in range(len(char_pool)):
        for j in range(len(char_pool)):
            for k in range(len(char_pool)):
                for l in range(len(char_pool)):
                    new_str = condition.decode("utf-8") + char_pool[i] + char_pool[j] + char_pool[k] + char_pool[l]
                    if simple_test(new_str):
                        r.sendline(new_str)
                        r.interactive()
                        sys.exit()

    r.close()

