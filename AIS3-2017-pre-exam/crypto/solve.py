encrypted = [964600246, 1376627084, 1208859320, 1482862807, 1326295511, 1181531558, 2003814564]
key = int("00001010001011011110100011110111", 2)

for e in encrypted:
    z = e ^ key
    z = str(bin(z))
    z = z[2:]
    for i in range(32 - len(z)):
        z = "0" + z
    print(chr(int(z[24:32], 2)), end="")
    print(chr(int(z[16:24], 2)), end="")
    print(chr(int(z[ 8:16], 2)), end="")
    print(chr(int(z[ 0: 8], 2)), end="")
        
