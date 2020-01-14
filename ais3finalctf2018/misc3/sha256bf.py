# name=e
# bmFtZT1l
# dd80518d39b23e7b0fa061fd1f7c4bca99cbd9c3a5f4d5de09b87fc98aadb6ec

import hashlib, string, itertools

v = b'name=e'
HH = 'dd80518d39b23e7b0fa061fd1f7c4bca99cbd9c3a5f4d5de09b87fc98aadb6ec'
i = 0
for chars in itertools.product(string.digits + string.ascii_letters, repeat=9):
	# key = i.to_bytes(9, 'little')
	key = ''.join(chars).encode()
	i += 1
	if i % 1000000 == 0: print(i, key)
	# print(key)
	h = hashlib.sha256( key+v ).hexdigest()
	# print(h)
	# print()
	# assert(len(h) == len(HH) and type(h) == type(HH))
	if hashlib.sha256( key+v ).hexdigest() == HH:
		print(i)
		print(key)
		print(h)
		break
