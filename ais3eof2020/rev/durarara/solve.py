
goal = [0] * 4
goal[0] = 0x7c8321db;
goal[1] = 0x7ca066ba;
goal[2] = 0x7c794ce6;
goal[3] = 0x7c78bc8a;

#  for k in range(256):
#      print(f'k={k}')
#      s = ''
#      for i in range(4):
#          x = goal[i] - 0x7c5d0f85
#          print(f'x={x}')
#          while x > 0:
#              print(x % 33, chr(x % 33 + k))
#              s += chr(x%33 + k)
#              x //= 33
#          print()
#      print(s)
#      break

def bruteforce(x):
    for a in range(33, 128):
        for b in range(33, 128):
            for c in range(33, 128):
                for d in range(33, 128):
                    y = 33**3*a + 33**2*b + 33*c + d
                    if y == x:
                        print(chr(a), chr(b), chr(c), chr(d))

for i in range(4):
    print(i)
    x = goal[i] - 0x7c5d0f85
    print(x)
#      bruteforce(x)
