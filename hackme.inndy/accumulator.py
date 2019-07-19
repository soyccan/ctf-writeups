import hashlib, random, string
s = b''
# print(hashlib.sha512(b'FLAG{}').hexdigest())
print(string.printable)
while True:
	s += string.printable[random.randint(0, 80)].encode()
	t = hashlib.sha512(b'FLAG{' + s + b'}').hexdigest()
	print(s, t)
	if t[0] == 0xc3: break
	if len(s) > 20: s = b''
