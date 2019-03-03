Files:
	chatserve.py - Python code for the chat server
	chatclient.c - C code for the chat client
	makefile - makefile for chatclient.c

Compilation:
	Place makefile and chatclient.c in the same directory and enter "make" on the command line create chatclient executable

Instructions:
	1. Start chatserve.py by entering "python3 chatserve.py [port]" on the command line, 
		- [port] is a valid numerical port number
	2. Start chatclient by entering "chatclient [server host] [server port]" on the command line
		- [server host] is the name of the server the chatserve.py program is running on
		- [server port] is the port chatserve.py is running on
	3. After connecting to the chat server, the client will prompt the user for a handle.  This handle is limited to ten characters
	4. After the chat server has sent a confirmation message to the client, the client can then send the first message. 
	5. The user on the server will then respond.
	6. The users on the client and server can exchange messages.
	7. Once either of the users enters "\quit", the chat session will end and the connection will close
	8. A new chat session can be initiated by repeating from step 2
	9. The chat server program be terminated at anytime by pressing CTRL+C
		- If the connection is still open, the server will send a "\quit" message to the client to terminate the connection

Note:
	These files were developed on flip3, and tested on flip1 and flip3
	
