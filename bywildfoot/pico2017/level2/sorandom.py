#!/usr/bin/python -u
import random,string

result = "BNZQ:8o149b15764q471k2533971t6w78liec"
#flag = "FLAG:"+open("flag", "r").read()[:-1]
#encflag = ""
flag = ""
UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
LOWER = "abcdefghijklmnopqrstuvwxyz"
digit = "0123456789"

for RRRRR in result:
    for TC in (UPPER + LOWER + digit + ":"):
        testflag = flag + TC
        encflag = ""
        random.seed("random")
        for c in testflag:
            if c.islower():
                encflag += chr((ord(c)-ord('a')+random.randrange(0,26))%26 + ord('a'))
            elif c.isupper():
                encflag += chr((ord(c)-ord('A')+random.randrange(0,26))%26 + ord('A'))
            elif c.isdigit():
                encflag += chr((ord(c)-ord('0')+random.randrange(0,10))%10 + ord('0'))
            else:
                encflag += c

        if encflag == result[0:len(encflag)]:
            print TC
            flag = flag + TC

print flag
