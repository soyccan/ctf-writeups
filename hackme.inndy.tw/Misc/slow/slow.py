import socket
import time

flag = "FLAG{"
#flag = "FLAG{222222_SLOOO"
test = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ"
def netcat(n):
    for x in test: 
        print( "Connecting..." )
        global flag
        target_host = "hackme.inndy.tw"
        target_port = 6688
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((target_host, target_port))

        time.sleep(1)

        data = client.recv(512)

        if data == b"":
            print( "Received nothing" )
        else:
            data = data.decode("utf-8")
            print("Received:", repr(data))

        spack = flag + x + "}\n"
        print("test : ", spack, end = "")
        client.sendall(bytes(spack, "utf-8")) #high-level to socket.send()

        time.sleep(5)

        data = client.recv(512)
        data = data.decode("utf-8")
        print("Received:", repr(data))
        if data[-4:-1] != "Bad":
            flag = flag + x
            print( "FLAG = ", flag + "}" )
            return;

        if len(data) > n + 12:
            flag = flag + x
            print(" Updata!! FLAG = ", flag + " }")
            netcat(n + 1)
            return

        print( "Connection closed." )
        #shutdown connection  SHUT_WR => further sends are disallowed
        client.shutdown(socket.SHUT_WR) 
        #releases the resource associated with a connention
        client.close()

netcat(0)

