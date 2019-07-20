import socket
import re
import time

def netcat(n):
	print( "Connecting..." )
	global flag
	target_host = "shell2017.picoctf.com"
	target_port = 26409
	client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client.connect((target_host, target_port))

	time.sleep(1)

	data = client.recv(512)

	if data == b"":
	    print( "Received nothing" )
	else:
	    data = data.decode("utf-8")
	    print("Received:", repr(data))

	result = re.findall(r"'.+?'", data)

	for i in range(3):
		result[i] = result[i][1:-1]
		print(result[i])

	return_string = ""
	for i in range(int(str(result[1]))):
		return_string = return_string + result[0]
	return_string = return_string + result[2] + "\n"

	print(return_string)

	client.send(return_string.encode())

	time.sleep(1)

	data = client.recv(512)

	if data == b"":
	    print( "Received nothing" )
	else:
	    data = data.decode("utf-8")
	    print("Received:", repr(data))

	print( "Connection closed." )
	client.shutdown(socket.SHUT_WR) 
	client.close()

netcat(0)
