# FLAG{W0w_BiiiiiiiiG_SiZe3e3!}
import sys


def run_command(context):
    """ run commands and get all assertions """
    def _context(): pass
    def _empty(): pass

    def _add(ctx, _):
        x = ctx.pop()
        y = ctx.pop()
        ctx.append(str(x) + '+' + str(y))

    def _sub(ctx, _):
        x = ctx.pop()
        y = ctx.pop()
        ctx.append(str(x) + '-' + str(y))

    def _cmp(ctx, num):
        x = ctx.pop()
        print('cmp', x, '==', num, x==num, file=sys.stderr)
        ctx.append(1)

    def _pop(ctx, _):
        ctx.pop()

    def _push(ctx, num):
        ctx.append(num)

    def _terminal(ctx, _):
        pass


    op = {0:_add,  1:_cmp,  2:_context,  3:_empty,  6:_pop,  7:_push,  8:_sub,  9:_terminal}
    raw_cmds = b'\x08\x00\x07\x08\x00\x00\x01d\t\x00\x00\x00\x014\t\x00\x073\x07\x01\x073\x08\x00\x00\x00\x01e\t\x00\x00\x00\x08\x00\x07c\x00\x00\x01\x00\t\x00\x00\x00\x074\x08\x00\x01\x00\t\x00\x06\x00\x01e\t\x00\x06\x00\x07Z\x08\x00\x01\x00\t\x00\x07h\x00\x00\x08\x00\x01\x00\t\x00\x06\x00\x07S\x08\x00\x01\x00\t\x00\x06\x00\x07_\x08\x00\x01\x00\t\x00\x06\x00\x07G\x08\x00\x01\x00\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01j\t\x00\x00\x00\x01C\t\x00\x06\x00\x07\x00\x07\x01\x00\x00\x07\x02\x00\x00\x07\x03\x00\x00\x07\x04\x00\x00\x07\x05\x00\x00\x07\x06\x00\x00\x07\x07\x00\x00\x07\x08\x00\x00\x07\t\x00\x00\x07\n\x00\x00\x07\x0b\x00\x00\x07\x0c\x00\x00\x07\r\x00\x00\x07\x04\x00\x00\x08\x00\x01\x00\t\x00\x06\x00\x01w\t\x00\x06\x00\x010\t\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x13\x00\x00\x01\x00\t\x00'

    cmds = list(zip(*(iter(raw_cmds),)*2))
    for c in cmds:
        print(str(op[c[0]]), c[1])
        op[c[0]](context, c[1])


# run_command() output, beautified:
#  || sub 0
#  || push 8
#  || add 0
#  || cmp 100
#  || terminal 0
#  || add 0
#  || cmp 52
#  || terminal 0
#  || push 51
#  || push 1
#  || push 51
#  || sub 0
#  || add 0
#  || cmp 101
#  || terminal 0
#  || add 0
#  || sub 0
#  || push 99
#  || add 0
#  || cmp 0
#  || terminal 0
#  || add 0
#  || push 52
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || pop 0
#  || cmp 101
#  || terminal 0
#  || pop 0
#  || push 90
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || push 104
#  || add 0
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || pop 0
#  || push 83
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || pop 0
#  || push 95
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || pop 0
#  || push 71
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 106
#  || terminal 0
#  || add 0
#  || cmp 67
#  || terminal 0
#  || pop 0
#  || push 0
#  || push 1
#  || add 0
#  || push 2
#  || add 0
#  || push 3
#  || add 0
#  || push 4
#  || add 0
#  || push 5
#  || add 0
#  || push 6
#  || add 0
#  || push 7
#  || add 0
#  || push 8
#  || add 0
#  || push 9
#  || add 0
#  || push 10
#  || add 0
#  || push 11
#  || add 0
#  || push 12
#  || add 0
#  || push 13
#  || add 0
#  || push 4
#  || add 0
#  || sub 0
#  || cmp 0
#  || terminal 0
#  || pop 0
#  || cmp 119
#  || terminal 0
#  || pop 0
#  || cmp 48
#  || terminal 0
#  || add 0
#  || add 0
#  || add 0
#  || add 0
#  || add 0
#  || add 0
#  || push 19
#  || add 0
#  || cmp 0
#  || terminal 0

# run_command output, with placeholder, beautified:
#  cmp 8+[28]-[27] == 100
#  cmp 1+[26] == 52
#  cmp 51-1+51 == 101
#  cmp 99+1+1-[25] == 0
#  cmp 52-1+[24] == 0 ( special case!!; should be 51-(1+[24]) )
#  cmp [23] == 101
#  cmp 90-[22] == 0
#  cmp 104+1-[21] == 0
#  cmp 83-[20] == 0
#  cmp 95-[19] == 0
#  cmp 71-[18] == 0
#  cmp 1+[17] == 106
#  cmp 1+[16] == 106
#  cmp 1+[15] == 106
#  cmp 1+[14] == 106
#  cmp 1+[13] == 106
#  cmp 1+[12] == 106
#  cmp 1+[11] == 106
#  cmp 1+[10] == 106
#  cmp 1+[9] == 67
#  cmp 4+13+12+11+10+9+8+7+6+5+4+3+2+1+0-[8] == 0
#  cmp [7] == 119
#  cmp [6] == 48
#  cmp 19+1+[5]+[4]+[3]+[2]+[1]+[0] == 0



def placeholder(context):
    for i in range(29):
        # placeholder for each index
        context.append('[{}]'.format(i))
    print(context)


# Guess initial context:
# b'FLAG{}' == [70, 76, 65, 71, 123, 125]
# (index value)
guess = '''
28 125
27 33
26 51
25 101
24 51
23 101
22 90
21 105
20 83
19 95
18 71
17 105
16 105
15 105
14 105
13 105
12 105
11 105
10 105
9 66
8 95
7 119
6 48
5 87
4 123
3 71
2 65
1 76
0 70
'''

def print_guessed_flag():
    context = [0] * 29
    for g in guess.split('\n'):
        if not g:
            continue
        index, value = g.split(' ')
        value = int(value)
        if value < 0:
            value += 256
        context[int(index)] = chr(int(value)).encode()
    print(b''.join(context))

context = []
placeholder(context)
run_command(context)
print_guessed_flag()

