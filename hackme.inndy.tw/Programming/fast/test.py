import socket
import time
def netcat():
    print ("starting connection")    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 12398))
    while True:
        time.sleep(3) 
        data = s.recv(128)
        if data == b"":
            pass
        else:
            data = data.decode("utf-8")
            print ("Received:", repr(data))

        user = input("what to send?: ")
        user = user + "\n"
        s.sendall(bytes(user, "utf-8"))

    print ("Connection closed.")
    s.shutdown(socket.SHUT_WR)
    s.close()
netcat()
