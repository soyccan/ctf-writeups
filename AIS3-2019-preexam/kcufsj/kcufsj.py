f = open('kcufsj', 'br')
fo = open('jsfuck', 'bw')
f.seek(0, 2)
i = 1
while True:
    if f.seek(-i, 2) <= 0:
        break
    fo.write(f.read(1))
    i += 1
fo.write(f.read(1))
