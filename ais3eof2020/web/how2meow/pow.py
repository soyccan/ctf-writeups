import hashlib, random, string, sys

while True:
    s = ''.join(random.choices(string.printable, k=10)).encode()
    if hashlib.md5(b'kaibro' + s).hexdigest()[:5] == sys.argv[1][:5]:
        print(s)
        break
