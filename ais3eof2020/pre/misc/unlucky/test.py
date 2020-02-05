import subprocess
f = open('/tmp/ebech2948', 'w')
f.write('abc')
f.flush()
print(f.fileno())
print(open('/proc/self/fdinfo/3', 'r').read())
p = subprocess.run(input(': ').split())
