import socket
import time

def netcat():
    print( "Connecting..." )
    target_host = "hackme.inndy.tw"
    target_port = 5500
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((target_host, target_port))

    solve = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    solve.connect(("127.0.0.1", 12398))
    
    time.sleep(5)
    data = client.recv(512)
    if data == b"":
        print( "Received nothing" )
    else:
        data = data.decode("utf-8")
        print("Received:", repr(data))
        print("Sending \"Yes I know...\"")
        spack = "Yes I know\n"
        client.sendall(bytes(spack, "utf-8")) #high-level to socket.send()
        
    while True:
        data = client.recv(128)
        if data == b"":
            print( "Received nothing" )
        else:
            data = data.decode("utf-8")
            print("Received:", repr(data))
            data = data[:-5]
            data = data + "\n"

        #spack = input("What to send? : ")
        #spack = spack + "\n"
        solve.sendall(bytes(data, "utf-8"))
        spack = solve.recv(512)
        print("Answer:", repr(spack))
        client.sendall(spack)

    print( "Connection closed." )
    #shutdown connection  SHUT_WR => further sends are disallowed
    client.shutdown(socket.SHUT_WR) 
    #releases the resource associated with a connention
    client.close()

netcat()

