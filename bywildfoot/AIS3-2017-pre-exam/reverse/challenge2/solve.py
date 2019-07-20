from subprocess import Popen, PIPE, STDOUT
import binascii

test_char = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`-=[];\'\\,./~!@#$%^&*()_+{}:\"|<>?"
flag = ""

f_c_elf = False

while f_c_elf == False:
    for c in test_char:
        test_flag = flag + c
        p = Popen(['./rev2_modify'], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        p.communicate(input=bytes(test_flag, "utf-8"))

        with open("./encrypted", "rb") as f:
            with open("./rev2_encrypted", "rb") as f_c:
                f_byte = f.read(1)
                flag_correct = True
                while f_byte != b"":
                    f_c_byte = f_c.read(1)
                    print(f_byte + f_c_byte)
                    if f_byte != f_c_byte:
                        flag_correct = False
                    f_byte = f.read(1)

                if flag_correct:
                    flag = flag + c

                if f_c.read(1) == b"":
                    f_c_elf = True

        print(flag)
