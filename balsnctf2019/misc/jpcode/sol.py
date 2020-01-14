from pwn import *

def toHEX(a):
    a = oct(a).replace('o', '')
    result = hex(0o200 + int(a[-2:], 8))[2:]
    result = hex(0o200 + int(a[-4:-2], 8))[2:] + result
    result = hex(0o340 + int(a[-6:-4], 8))[2:] + result

    return bytes.fromhex(result)

def getJapList():
    jap = list(range(0x3041, 0x3096+1))     # U+3041 ~ U+3096
    jap += list(range(0x3099, 0x309f+1))    # U+3099 ~ U+309F
    jap += list(range(0x30a0, 0x30ff+1))    # U+30A0 ~ U+30FF
    jap += list(range(0x3190, 0x319f+1))    # U+3190 ~ U+319F
    jap += list(range(0x31f0, 0x31ff+1))    # U+31F0 ~ U+31FF
    jap += list(range(0xff66, 0xff9f+1))    # U+FF66 ~ U+FF9F

    japlist = []

    for i in jap:
        japlist.append(toHEX(i))

    return japlist

def checkInput(payload):
    japlist = getJapList()

    if len(payload) == 0 or len(payload) % 3 != 0:
        print('3-align')

    for i in range(len(payload)//3):
        if payload[i*3:i*3+3] not in japlist:
            print('Error', payload[i*3:i*3+3])


context(arch='i386', os='linux')

#  code = shellcraft.sh()
#  sc = asm(code)
#  sc = b'\x90' + sc
#  print(sc)
#  getJapList()
#  checkInput(sc)
ins = []
japlist = getJapList()
for i in japlist:
    a = disasm(i)
    for b in a.split('\n'):
        if '.' not in b:
            ins.append(b[8:])
ins = list(set(ins))
print('\n'.join(ins), file=open('ins', 'w'))

