import numpy, subprocess, typing

class Node:
    def __init__(self, num):
        self.num = num


nodes = [ [] for _ in range(50) ]
tree = [ numpy.empty(10000, dtype=numpy.int32) for _ in range(50) ]

def build(pos, idx, depth):
    if depth > 11:
        return

    tree[pos][idx] = len(nodes[pos])
    nd = Node(0)
    nodes[pos].append(nd)

    build(pos, idx*2, depth+1)
    build(pos, idx*2+1, depth+1)

def insert(pos, idx, path, num, depth):
    if depth == len(path) - 1:
#          print('set', idx, path[depth], num, depth)
        nodes[pos][tree[pos][idx]].num = num
        return

    print('in', pos, idx, path[depth], num, depth)


    next_bit = path[depth+1].split('_')[0]
    if next_bit == '0':
        insert(pos, idx*2, path, num, depth+1)

    elif next_bit == '1':
        insert(pos, idx*2+1, path, num, depth+1)

    else:
        raise



    op = path[depth].split('_')[1]

    if op == '+':
        nodes[pos][tree[pos][idx]].num = nodes[pos][tree[pos][idx*2]].num + nodes[pos][tree[pos][idx*2+1]].num
    elif op == 'x':
        nodes[pos][tree[pos][idx]].num = nodes[pos][tree[pos][idx*2]].num * nodes[pos][tree[pos][idx*2+1]].num
    else:
        raise

#      print('out', pos, idx, path[depth], num, depth)



for i in range(50):
    build(i, 1, 0)

for line in open('files.1'):
    line = line.strip('\n')
    if line.endswith('/'): continue
    line = line.replace('[', '\\[').replace(']', '\\]')

    l = line.strip('\n').split('/')

    pos = l[0].replace('flag\\[', '').replace('\\]', '')
    try:
        pos = int(pos)
    except:
        continue
    print('line', line)
    print('pos', pos)
    p = subprocess.run(['unzip', '-p', 'Tree.zip', line], stdout=subprocess.PIPE)
    num = int(p.stdout.decode())

    insert(pos, 1, l[1:], num, 0)


res = [0] * 50
for i in range(50):
    print('sum', i, nodes[i][tree[i][1]].num)
    res[i] = nodes[i][tree[i][1]].num
print(res)




