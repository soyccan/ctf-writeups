# pipe in pusheen_decode

binary = b""

while True:
    s = ""
    try:
        s = input()
    except:
        break
    if s[1] == ' ':
        #print("0", end = "")
        binary = binary + b"0"
    else:
        #print("1", end = "")
        binary = binary + b"1"

decoded = int(binary, 2)
print(decoded.to_bytes((decoded.bit_length() + 7) // 8, "big").decode())
