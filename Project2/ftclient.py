#####################################################################################################
#  Course NameL CS 372 Introduction to Computer Networks
#  Programming Assignment #1
#  Program Name: chatserve.py
#  Programmer: Joshua Nutt
#  Last Modified: 2/12/2019
#  Description: This program is a chat server.  Is establishes a TCP socket and lets a client program
#  connect with it.  The server and client can then chat back and forth with eachother.  The 
#  client or server can terminate the chat session.  Once the session is closed, the  server listens
#  for a new connection from a client.  The server can be terminated by Ctrl+C
#  Sources: Computer Networking: A Top-Down Approach 7th Edition by Kurose and Ross pgs 164-169
######################################################################################################

import sys #to access args and exit
from socket import * #to use sockets

def receiveMessage(dataSocket):
	ack = "ACK"
	message = dataSocket.recv(500).decode()
	dataSocket.send(ack.encode())
	return message;
	
def sendMessage(dataSocket, message):
	dataSocket.send(message.encode())
	dataSocket.recv(500).decode()
	
def receiveDirectory(dataSocket):
	#ack = "ACK"
	print("Receiving directory structure from " + sys.argv[1] + ":" + sys.argv[4])
	filename = receiveMessage(dataSocket)
	#filename = dataSocket.recv(500).decode()
	#dataSocket.send(ack.encode())
	while filename != "***FIN":
		print(filename, end="\n")
		filename = receiveMessage(dataSocket)
		#filename = dataSocket.recv(500).decode()
		#dataSocket.send(ack.encode())

# make sure three arguments were entered when calling this program
#if len(sys.argv) != 3:
#	print('Usage: chatserve.py requires a port number')
#	sys.exit(0)

serverName = sys.argv[1] + ".engr.oregonstate.edu"
serverPort = int(sys.argv[2])
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName,serverPort))
print("Client Connected")
portno = ""
command = sys.argv[3]
if command == "-l":
	portno = sys.argv[4]
elif command == "-g":
	portno = sys.argv[5]
else:
	sys.exit(0)
#clientSocket.send(portno.encode())
sendMessage(clientSocket, portno)
#clientSocket.send(command.encode())
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
	else:
		print(message)

if command == "-l":
	clientPort = int(portno)
	serverSocket = socket(AF_INET, SOCK_STREAM)
	try:
		serverSocket.bind(('',clientPort))
		serverSocket.listen(1)
		dataSocket, addr = serverSocket.accept()
		receiveDirectory(dataSocket)
		dataSocket.close() #close data connection
		print("Closed data socket")
	except:
		print("ERROR: Port already in use")
	serverSocket.close(); #close listening socket
	print("Closed Server Socket")
	

#message = dataSocket.recv(500).decode()
#print(message)

#dataSocket.close();

clientSocket.close(); #close original connecting socket
print("Closed client Socket")

########################################################################################
# Description: Creates a IPv4 TCP socket and sets it to listen for connections
# Parameters: The chat server's port number as a string
# Returns: The server socket
# Pre-conditions: a port number has been selected
# Post-conditions: The listening socket has been created
########################################################################################
#def createSocket(port):
#	serverPort = int(port) #get the port number and convert from string to int
#	serverSocket = socket(AF_INET, SOCK_STREAM) #set up an IPv4 TCP socket
#	serverSocket.bind(('', serverPort)) #assign the port number to the socket
#	serverSocket.listen(1) #listen for TCP connection requests
#	return serverSocket
#
########################################################################################
# Description: Send a message to the chat client
# Parameters: The server's handle, the connection socket
# Returns: The message that was sent to the client
# Pre-conditions: A connecton has been established
# Post-conditions: A message has been sent
########################################################################################
#def sendMessage(handle, connectionSocket):
#	message = input(handle) #get server's response to client
#	if message != "\quit": #if the server has not decided to quit
#		message = handle + message #prepend server's handle to message
#	else: #if server has decided to quit, print termination message
#		print('Terminating session')
#	connectionSocket.send(message.encode()) #send message to client
#	return message

########################################################################################
# Description: Receives a message from the chat client, then offers user a chance to
# send a messagee in response
# Parameters: The server's handle, the connection socket
# Returns: The last message that was sent or received (so it can be tested to see if
# either of the user's want to quit)
# Pre-conditions: A connecton has been established
# Post-conditions: A message has been received
########################################################################################
#def recvMessage(handle, connectionSocket):
#	message = connectionSocket.recv(500).decode() #receive message from client
#	if message == '\quit':  #Print warning termination message if client quits
#		print('Session terminated by client')
#	else: #if client has not quit
#		print(message) #print message from client
#		message = sendMessage(handle, connectionSocket)
#	return message

########################################################################################
# Description: Loops listening for connections until the user hits CTRL+C to terminate
# the chat serve program
# Parameters: The server's handle, the listening socket
# Pre-conditions: The serve socket is listening for connections
# Post-conditions: The user has hit CTRL+C to terminate program
########################################################################################
#def connectLoop(handle, serverSocket):
#	while True: #loop until SIGINT is received
#		print('\nThe server is ready to receive\n')
#		connectionSocket, addr = serverSocket.accept() #create a socket for TCP request from client
#		message = connectionSocket.recv(500).decode() #get portnum from client
#		message = handle + 'Chat session opened.' #Alert client that session has been opened
#		connectionSocket.send(message.encode())
#		print(message)
#		while message != '\quit':  #loop until server of client types "\quit"
#			message = recvMessage(handle, connectionSocket)
#		connectionSocket.close()



########################################################################################
# Description: Main body of the program
########################################################################################
# make sure two arguments were entered when calling this program
#if len(sys.argv) != 2:
#	print('Usage: chatserve.py requires a port number')
#	sys.exit(0)

#Set up SIGINT handler for CTRL+C
#Source: https://stackoverflow.com/questions/1112343/how-do-i-capture-sigint-in-python 
#def signal_handler(sig, frame):
#	print('\nTerminating ' + sys.argv[0])
#	sys.exit(0)
#signal.signal(signal.SIGINT, signal_handler)

#serverSocket = createSocket(sys.argv[1]) #create the listening socket
#handle = "Server> " #set the server's handle
#connectLoop(handle, serverSocket) 
