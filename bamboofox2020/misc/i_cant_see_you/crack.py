import subprocess

n = 4
head = b'BAMBOOFOX{'
guess = bytearray(n)

def backtrack(i):
    if i >= n:
        return


    for j in range(25):
        guess[i] = ord('A') + j

        p = head + guess[:i+1].strip(b'\0') + b'}'
        p = p.decode()
        print(p)

        r = subprocess.run(['rar', 'x', '-p' + p, 'what.rar'], stdout=None, stderr=None)
        if r.returncode == 0:
            break

        backtrack(i+1)

backtrack(0)
