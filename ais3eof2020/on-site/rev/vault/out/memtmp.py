f = open('mem.txt.out', 'w')
for line in open('mem.txt'):
    try:
        a, _, v = line.partition(':')
        if not v:
            raise ValueError
        a = int(a)
        v = int(v)
        f.write('{}: {} {}\n'.format(a, hex(v), v.to_bytes(1, 'little')))
    except ValueError:
        f.write(line)

