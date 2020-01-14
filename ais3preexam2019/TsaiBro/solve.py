f = open('flag.txt', 'br')
f.readline()
ln = f.readline()

print(ln)
i = 6
cnt = 0
arr = []
while i < len(ln):
    # print(hex(ln[i]))
    if ln[i] != ord('.'):
        arr.append(cnt)
        cnt = 0
        i += 6
    else:
        cnt += 1
        i += 1
arr.append(cnt)
print(arr)
print(len(arr))

brr = []
i = 0
while i < len(arr):
    brr.append((arr[i] - 1) * 8 + (arr[i+1] - 1))
    i += 2
print(brr)


CHR = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWQY0123456789{}_'
# no Z
print(''.join(CHR[i] for i in brr))
