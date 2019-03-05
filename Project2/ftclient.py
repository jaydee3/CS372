#####################################################################################################
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
		print("USAGE: invalid number of arguments\n")
		sys.exit(1)
	if sys.argv[1] != "flip1" and  sys.argv[1] != "flip2" and sys.argv[1] != "flip3":
		print("USAGE: Second argument must be flip1, flip2, or flip3\n")
		sys.exit(1)
	if int(sys.argv[2]) < 1024 or int(sys.argv[2]) > 65535:
		print("USAGE: Invalid port number at argument three\n")
		sys.exit(1)
	if sys.argv[3] != "-g" and sys.argv[3] != "-l":
		print("USAGE: Fourth argument must be '-l' to list files or '-g' to get file\n")
		sys.exit(1)
	if sys.argv[3] == "-l": 
		if int(sys.argv[4]) < 1024 or int(sys.argv[4]) > 65535:
			print("USAGE: Invalid port number at argument five\n")
			sys.exit(1)
		if len(sys.argv) != 5:
			print("USAGE: Invalid number of arguments for command '-l'\n")
			sys.exit(1)
	if sys.argv[3] == "-g": 
		if int(sys.argv[5]) < 1024 or int(sys.argv[5]) > 65535:
			print("USAGE: Invalid port number at argument six\n")
			sys.exit(1)
		if len(sys.argv) != 6:
			print("USAGE: Invalid number of arguments for command '-g'\n")
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
	# Receive the size of the file as a string
	raw_msglen = receiveMessage(sock)
	#if not received, return null string
	if not raw_msglen:
		return None
	#onvert received number to integer
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
    while len(data) < n: #n is the total number of expected bytes that will be transfered
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
	print("Receiving directory structure from " + sys.argv[1] + ":" + sys.argv[4])
	filename = receiveMessage(dataSocket)
	while filename != "***FIN":
		print("  " + filename, end="\n")
		filename = receiveMessage(dataSocket)
	print("\n")

########################################################################################
# Description: Creates a file that the user has requested, then fills that file with
# incoming data from the server
# Parameters: The socket the data will be sent over
# Pre-conditions: The socket is opened, the file exists on the server, and the file
# contents are beinf sent over the server
# Post-conditions: The file has been copied from the server
########################################################################################
def receiveFile(dataSocket):
	i = 0; #this counter is appended to the end of the filename if the file already exists in the client's folder
	filename = sys.argv[4]
	print("Receiving \"" + filename +"\" from " + sys.argv[1] + ":" + sys.argv[5])
	#Source: https://therenegadecoder.com/code/how-to-check-if-a-file-exists-in-python/
	config = Path(filename)
	while config.is_file(): #while a file with the stored filename already exists
		i += 1; #if it exists, increment the counter
		#Source: https://stackoverflow.com/questions/4022827/insert-some-string-into-given-string-at-given-index-in-python
		filename = sys.argv[4]
		index = filename.find('.') #see if there is a dot in the file name
		if index == -1: # if there is no dot in the file name
			filename = filename + "_" + str(i) #append the number in the counter to the filename
		else: #if there is a .
			filename = filename[:index] + "_" + str(i) + filename[index:] #append the counter number before the dot
		config = Path(filename) #store the new file name in config to test on the next loop
	if filename != sys.argv[4]: #if the file has to be saved as a copy
		print("File " + sys.argv[4] + " already exists. Saving file as " + filename)
		
	#Source: https://www.guru99.com/reading-and-writing-files-in-python.html
	filedesc = open(filename,"wt") #open file for writing
	filebuffer = recv_msg(dataSocket) #receive data
	filedesc.write(filebuffer) #save data to file
	filedesc.close() #close file
	print("File transfer complete\n")

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
	#get port no based on command entered on command line
	if command == "-l":
		portno = sys.argv[4]
	elif command == "-g":
		portno = sys.argv[5]
	else:
		sys.exit(0) #exit if invlaid command
	sendMessage(clientSocket, portno) #send data port to server
	sendMessage(clientSocket, command) #send command to server

	#if the user requested a file, send filename to client and exit if file was not found
	if command == "-g":
		filename = sys.argv[4]
		sendMessage(clientSocket, filename)
		message = receiveMessage(clientSocket)
		if message == "File not found":
			print(sys.argv[1] + ":" + sys.argv[2] + " says FILE NOT FOUND\n")
			clientSocket.close(); #close original connecting socket
			sys.exit(0)
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
	#create, bind, and listen for connection with data connection socket
	serverSocket = socket(AF_INET, SOCK_STREAM)
	try: #sometimes the ports are not available, so exit if not
		serverSocket.bind(('',clientPort))
		serverSocket.listen(1)
		dataSocket, addr = serverSocket.accept()
	except:
		print("ERROR: Port already in use\n")
		serverSocket.close(); #close listening socket
		clientSocket.close(); #close original connecting socket
		sys.exit(0)	
	
	#Receive file list
	if command == "-l":
		receiveDirectory(dataSocket)

	#recive file
	if command == "-g":
		receiveFile(dataSocket)

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
