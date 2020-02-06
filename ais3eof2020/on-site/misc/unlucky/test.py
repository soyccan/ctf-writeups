import subprocess, os
f = open('/tmp/ebech2948', 'w')
f.write('abc')
f.flush()
print(f.fileno())
print(open('/proc/self/fdinfo/3', 'r').read())
#  cmd = 'cat /proc/{}/fd/3'.format(os.getppid())

p = subprocess.Popen(input(': ').split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
r = p.communicate()
print(r[0].decode())
print(r[1].decode())

p = subprocess.Popen(input(': ').split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
r = p.communicate()
print(r[0].decode())
print(r[1].decode())
