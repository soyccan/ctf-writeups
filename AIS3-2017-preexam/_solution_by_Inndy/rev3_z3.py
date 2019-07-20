from z3 import *

known = [
    [ 0x11, 0x00, 0x00, 0x0A, 0x00 ],
    [ 0x00, 0x00, 0x06, 0x00, 0x12 ],
    [ 0x01, 0x00, 0x00, 0x00, 0x00 ],
    [ 0x00, 0x00, 0x00, 0x15, 0x02 ],
    [ 0x00, 0x10, 0x00, 0x00, 0x00 ]
]
arr = [ [] for i in range(5) ]
v = []
s = Solver()

for i in range(5):
    for j in range(5):
        if not known[i][j]:
            x = Int('v%d,%d' % (i, j))
            s.add(0 < x)
            s.add(x <= 25)
            arr[i].append(x)
            v.append(x)
        else:
            arr[i].append(known[i][j])

for i in v:
    for j in v:
        if i is not j:
            s.add(i != j)

print(s.check())
