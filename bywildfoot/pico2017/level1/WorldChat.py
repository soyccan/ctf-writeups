import socket
import re
import time

def netcat(n):
	print( "Connecting..." )
	global flag
	target_host = "shell2017.picoctf.com"
	target_port = 38798
	client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client.connect((target_host, target_port))

	flag_group = []
	while 1:
		time.sleep(1)

		data = client.recv(512)

		if data == b"":
			print( "Received nothing" )
		else:
			data = data.decode("utf-8")
			#print("Received:", repr(data))

		result = re.findall(r"flagp.*\n", data)
		for j in result:
			flag_group.append( j )
		print(result)
		if len(flag_group) == 8:
			break
		

	print( flag_group )
	print( "Connection closed." )
	client.shutdown(socket.SHUT_WR) 
	client.close()

netcat(0)
