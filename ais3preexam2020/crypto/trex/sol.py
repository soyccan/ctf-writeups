# AIS3{TYR4NN0S4URU5_R3X_GIV3_Y0U_SOMETHING_RANDOM_5TD6XQIVN3H7EUF8ODET4T3H907HUC69L6LTSH4KN3EURN49BIOUY6HBFCVJRZP0O83FWM0Z59IISJ5A2VFQG1QJ0LECYLA0A1UYIHTIIT1IWH0JX4T3ZJ1KSBRM9GED63CJVBQHQORVEJZELUJW5UG78B9PP1SIRM1IF500H52USDPIVRK7VGZULBO3RRE1OLNGNALX}
M = []
lines = open('prob').read().split('\n\n')
for l in lines:
    M.append(l.split(' '*7))
T = {}
for i in range(1, len(M)):
    for j in range(1, len(M[i])):
        T[(M[0][j] + M[i][0]).strip(' ')] = M[i][j]
print(T)

res = ''
for c in M[-1][0].split(' '):
    d = T.get(c, None)
    print(f'{c=} {d=}')
    if d:
        res += d
    else:
        res += c
print(res)
