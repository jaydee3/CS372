i#####################################################################################################
#  Course Name: CS 372 Introduction to Computer Networks
#  Project 2
#  Program Name: ftclient.py
#  Programmer: Joshua Nutt
#  Last Modified: 3/4/2019
#  Description: This program is a FTP client.  It connected with an FTP server through a TCP socket
#  and then requests a list of files or to transfter a single file.  It then sets up a listening
#  socket, which the srver will then connect with to send the file listing or file through. The server
#  can be terminated by Ctrl+C
#  Sources: Computer Networking: A Top-Down Approach 7th Edition by Kurose and Ross pgs 164-169
######################################################################################################

import sys #to access args and exit
from socket import * #to use sockets
from pathlib import Path #to test if a file exists

########################################################################################
# Description: Validates the agruments entered on teh command line
# Parameters: The arguments entered on the command line
# Pre-conditions: Arguments have been entereed on command line
# Post-conditions: The arguments have been validated or the program terminated if
# any of the aruments are not valid
########################################################################################
def checkArgs():
	if len(sys.argv) < 5 or len(sys.argv) > 6:
		print("USAGE: invalid number of arguments")
		sys.exit(1)
	if sys.argv[1] != "flip1" and  sys.argv[1] != "flip2" and sys.argv[1] != "flip3":
		print("USAGE: Second argument must be flip1, flip2, or flip3")
		sys.exit(1)
	if int(sys.argv[2]) < 1024 or int(sys.argv[2]) > 65535:
		print("USAGE: Invalid port number at argument three")
		sys.exit(1)
	if sys.argv[3] != "-g" and sys.argv[3] != "-l":
		print("USAGE: Fourth argument must be '-l' to list files or '-g' to get file")
		sys.exit(1)
	if sys.argv[3] == "-l": 
		if int(sys.argv[4]) < 1024 or int(sys.argv[4]) > 65535:
			print("USAGE: Invalid port number at argument five")
			sys.exit(1)
		if len(sys.argv) != 5:
			print("USAGE: Invalid number of arguments for command '-l'")
			sys.exit(1)
	if sys.argv[3] == "-g": 
		if int(sys.argv[5]) < 1024 or int(sys.argv[5]) > 65535:
			print("USAGE: Invalid port number at argument six")
			sys.exit(1)
		if len(sys.argv) != 6:
			print("USAGE: Invalid number of arguments for command '-g'")
			sys.exit(1)
		
########################################################################################
# Description: Used to receive a large stream such as a file from the server
# Parameters: The socket the file will be sent over
# Returns: The string that was received from the server
# Pre-conditions: The socket is opened
# Post-conditions: The message has been received
# Source: https://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data		
########################################################################################

def recv_msg(sock):
    # Read message length and unpack it into an integer
    raw_msglen = receiveMessage(sock)
    if not raw_msglen:
        return None
    msglen = int(raw_msglen)
    # Read the message data
    return recvall(sock, msglen)

########################################################################################
# Description: Helper function that receives large stream such as a file from the server
# Parameters: The socket the file will be sent over, the number of bytes that will be 
# received
# Returns: The string that was received from the server
# Pre-conditions: The socket is opened
# Post-conditions: The message has been received
# Source: https://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data		
########################################################################################
def recvall(sock, n):
    # Helper function to recv n bytes or return None if EOF is hit
    data = ''
    while len(data) < n:
        packet = sock.recv(n - len(data)).decode()
        if not packet:
            return None
        data += packet
    return data

########################################################################################
# Description: Receives a message from the server then sends an acknowledgement back
# Parameters: The socket the message will be sent over
# Returns: The received message
# Pre-conditions: The socket is opened and a message will be sent over the socket
# Post-conditions: The message has been received
########################################################################################
def receiveMessage(dataSocket):
	ack = "ACK"
	message = dataSocket.recv(500).decode()
	dataSocket.send(ack.encode())
	return message;
	
########################################################################################
# Description: Sends a message then receives an acknowledgement the message was sent
# Parameters: The socket the message will be sent over, the message
# Pre-conditions: The socket is opened
# Post-conditions: The message has been sent
########################################################################################
def sendMessage(dataSocket, message):
	dataSocket.send(message.encode())
	dataSocket.recv(500).decode()
	
########################################################################################
# Description: Receives a list of files from the client
# Parameters: The socket the list will be sent over
# Pre-conditions: The socket is open and a directory is being sent over the socket
# Post-conditions: The directory is received
########################################################################################
def receiveDirectory(dataSocket):
	#ack = "ACK"
	print("Receiving directory structure from " + sys.argv[1] + ":" + sys.argv[4])
	filename = receiveMessage(dataSocket)
	while filename != "***FIN":
		print(filename, end="\n")
		filename = receiveMessage(dataSocket)

########################################################################################
# Description: Creates a file that the user has requested, then fills that file with
# incoming data from the server
# Parameters: The socket the data will be sent over
# Pre-conditions: The socket is opened, the file exists on the server, and the file
# contents are beinf sent over the server
# Post-conditions: The file has been copied from the server
########################################################################################
def receiveFile(dataSocket):
	i = 0;
	filename = sys.argv[4]
	config = Path(filename)
	while config.is_file():
		i += 1;
		#Source: https://stackoverflow.com/questions/4022827/insert-some-string-into-given-string-at-given-index-in-python
		filename = sys.argv[4]
		index = filename.find('.')
		if index == -1:
			filename = filename + "(" + str(i) + ")"
		else:
			filename = filename[:index] + "(" + str(i) + ")" + filename[index:]
		config = Path(filename)
	if filename != sys.argv[4]:
		print("File " + sys.argv[4] + " already exists. Saving file as " + filename)
		
	#Source: https://www.guru99.com/reading-and-writing-files-in-python.html
	filedesc = open(filename,"wt")
	filebuffer = recv_msg(dataSocket)
	filedesc.write(filebuffer)
	filedesc.close()

########################################################################################
# Description: Creates a control connection with the server
# Returns: The socket that has established a connection with the server
# Pre-conditions: a server name and port number has been entered on the command line
# Post-conditions: The connection has been established
########################################################################################
def initiateContact():
	serverName = sys.argv[1] + ".engr.oregonstate.edu"
	serverPort = int(sys.argv[2])
	clientSocket = socket(AF_INET, SOCK_STREAM)
	clientSocket.connect((serverName,serverPort))
	print("Client Connected")
	return clientSocket


########################################################################################
# Description: Sends a requst to the client
# Parameters: The socket the request will be sent over
# Returns: The port number that the requested data will be sent over
# Pre-conditions: a command and port number have been enteded on the command line
# Post-conditions: The command has been sent to the server
########################################################################################
def makeRequest(clientSocket):
	portno = ""
	command = sys.argv[3]
	if command == "-l":
		portno = sys.argv[4]
	elif command == "-g":
		portno = sys.argv[5]
	else:
		sys.exit(0)
	sendMessage(clientSocket, portno)
	sendMessage(clientSocket, command)
	print("Send portno and command")

	if command == "-g":
		filename = sys.argv[4]
		sendMessage(clientSocket, filename)
		message = receiveMessage(clientSocket)
		if message == "File not found":
			print(message)
			clientSocket.close(); #close original connecting socket
			sys.exit(0)
	#	else:
	#		print(message)
	return portno

########################################################################################
# Description: Receive a file list or file from the server
# Parameters: The port number that will be used to establish the data connections
# Pre-conditions: a port number has been selected
# Post-conditions: The data has been received
########################################################################################
def receiveData(portno):
	command = sys.argv[3]
	clientPort = int(portno)
	serverSocket = socket(AF_INET, SOCK_STREAM)
	try:
		serverSocket.bind(('',clientPort))
		serverSocket.listen(1)
		dataSocket, addr = serverSocket.accept()
	except:
		print("ERROR: Port already in use")
		serverSocket.close(); #close listening socket
		clientSocket.close(); #close original connecting socket
		sys.exit(0)	

	if command == "-l":
		receiveDirectory(dataSocket)
		dataSocket.close() #close data connection
		print("Closed data socket")

	if command == "-g":
		receiveFile(dataSocket)
		dataSocket.close() #close data connection
		print("Closed data socket")

	serverSocket.close(); #close listening socket

########################################################################################
# Description: This is the main loop of the program
# Parameters: The arguments entered on the command line
# Pre-conditions: The correct arguments have been entered on the command line
# Post-conditions: The request data has been received from the server
########################################################################################
checkArgs() #validate command line argument
clientSocket = initiateContact() #establish control connections
portno = makeRequest(clientSocket) #send request to server
receiveData(portno) #receive data from server
clientSocket.close(); #close original connecting socket
