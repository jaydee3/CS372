from socket import *
#serverName = 'flip3.engr.oregonstate.edu'
serverPort = 63894
clientSocket = socket(AF_INET, SOCK_STREAM)
#clientSocket.connect((serverName,serverPort))
clientSocket.connect(('',serverPort))
handle = input('Enter your handle: ')
sentence = "start"
while sentence != "\quit":
	sentence = input('> ')
	#sentence = handle + '> ' + sentence
	clientSocket.send(sentence.encode())
	modifiedSentence = clientSocket.recv(500)
	print(modifiedSentence.decode())
clientSocket.close()
