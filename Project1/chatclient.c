/**************************************************************************************************************************
#  Course NameL CS 372 Introduction to Computer Networks
#  Programming Assignment #1
#  Program Name: chatclient
#  Programmer: Joshua Nutt
#  Last Modified: 2/12/2019
#  Description: This program is a chat client.  It establishes a socket and creats a TCP connection
#  to a chat server.  The server and client can then chat back and forth with eachother.  The
#  client or server can terminate the chat session by entering "\quit" at the prompt.  Once the session
#  is closed, the server listens for a new connection from a client.
#  Sources: Beej's Guide to Network Programming Using Internet Sockets: https://beej.us/guide/bgnet/html/single/bgnet.html
# 	    Oregon State CS 344 lectures 4.2 Network Clients & 4.3 Network Servers by Benjamin Brewster
***************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h> 

void removeNewline(char* ary, int size);
void getHandle(char* handle, int size);
void initiateConnection(int socketFD, char* port, struct sockaddr_in *serverAddress, int size);
void sendMessage(int socketFD, char* handle, int *quitFlag);
void message(int socketFD, char* handle);

/************************************************************************************
 * Description: Main function of the program
 * Parameters: the number of arguments from the command line, the arguments from
 * the command line
 * Pre-conditions: The server address and port number should be entered as 
 * arguments on the command line
 * Post-conditions: Program has terminated
 * *********************************************************************************/
int main(int argc, char *argv[])
{
	if (argc != 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	int socketFD, portNumber; //used for connecting to server
	struct sockaddr_in serverAddress; //used for connection to server
	struct hostent* serverHostInfo; //used for connecting to server
	char handle[11]; //ten characters and null terminator

	//set up server address structure
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	getHandle(handle, sizeof(handle)); //get user's name

	//set up socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	initiateConnection(socketFD, argv[2], &serverAddress, sizeof(serverAddress));

	
	message(socketFD, handle); //call function to receive and then send message

	close(socketFD);

	return 0;
}

/************************************************************************************
 * Description: Replaces newline characters inserted by fgets with null terminators
 * Parameters: A pointer to a char array, the size of the array
 * Pre-conditions: The char array has been declared
 * Post-conditions: all new-line characters are replaced with null terminators
 * *********************************************************************************/
void removeNewline(char* ary, int size)
{

	//remove newline character appended by fgets
	int i;
	for (i = 0; i < size; i++)
	{	
		if (ary[i] == '\n')
			ary[i] = '\0';
	}
}

/***********************************************************************
 * Description: Gets the handle from the user
 * Parameters: A pointer to a char array, the size of the array
 * Pre-conditions: The char array has been declared
 * Post-conditions: the char array contains the user's handle
 * ********************************************************************/
void getHandle(char* handle, int size)
{
	printf("\nEnter your handle> "); //prompt
	fgets(handle, 11, stdin); //the user can enter up to 10 characters
	removeNewline(handle, size); //remove the newline appended by fgets
	setbuf(stdin, NULL); //flush the buffer
}


/***********************************************************************
 * Description: Initiate a connection with the server
 * Parameters: A socket to use to connect, the clients port number, the
 * server's address structure, the size of the address structure
 * Pre-conditions: The socket has been created
 * Post-conditions: the connection to the server has been established
 * ********************************************************************/
void initiateConnection(int socketFD, char* port, struct sockaddr_in *serverAddress, int size)
{
	//connect to the server
	if (connect(socketFD, (struct sockaddr*)serverAddress, size) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	char buffer[10];
	//send port number
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	strcpy(buffer, port); //create authentication message
	int charsSent = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsSent < 0) error("CLIENT: ERROR writing to socket");
	if (charsSent < strlen(buffer)) fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
}



/***********************************************************************
 * Description: Send a message to the server
 * Parameters: A socket to use to connect, the user's handle, a flag
 * that tells if the user's have chosen to terminate the chat
 * Pre-conditions: a socket connection has been established
 * Post-conditions: A message has been sent to the server and the user
 * has determined whether to quit or not
 * ********************************************************************/
void sendMessage(int socketFD, char* handle, int *quitFlag)
{
	char buffer[500];
	char input[500];

	//send message to server
	memset(buffer, '\0', sizeof(buffer)); //Clear out the buffer array
	memset(input, '\0', sizeof(input)); //Clear out the input again for reuse
	printf("%s> ", handle); //display the prompt
	fgets(input, 500 - strlen(handle) + 2, stdin); //get the message from user, but leave room to prepend the handle
	removeNewline(input, sizeof(input)); //replace newline characters from fgets with null terminators	
			
	if(strcmp(input, "\\quit")) //if the user did not enter "\quit", prepend the handle to the message
	{
		strcat(buffer, handle);
		strcat(buffer, "> ");
	}
	else //if the user did choose to quit
	{
		*quitFlag = 1; //set flag to one so program knows to terminate
		fprintf(stdout, "%s\n\n", "Terminating session");
	}

	strcat(buffer, input); //send the message whether the user chose to quit or not
	int charsSent = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsSent < 0) error("CLIENT: ERROR writing to socket");
	if (charsSent < strlen(buffer)) fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
}


/***********************************************************************
 * Description: Receives message from server and calls function to 
 * send message
 * Parameters: A socket to use to connect, the user's handle
 * Pre-conditions: A socket conneection has been established
 * Post-conditions: A message has been received from the server, and the
 * user has had a chance to send a message
 * ********************************************************************/
void message(int socketFD, char* handle)
{
	int quitFlag = 0;//set to 1 when the client or server enters "\quit"

	while(!quitFlag)
	{
		char buffer[500];

		//get message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		int charsRecv = recv(socketFD, buffer, sizeof(buffer), 0); // Read data from the socket
		if (charsRecv < 0) error("CLIENT: ERROR reading from socket");

		if(!strcmp(buffer, "\\quit")) //if the server sent "\quit"
		{
			fprintf(stdout, "%s\n\n", "Session terminated by server");
			quitFlag = 1; //set flag to 1 so program knows to terminate
		}
		else //if the server did not "\quit"
		{
			fprintf(stdout, "%s\n", buffer); //print the message sent by server
			sendMessage(socketFD, handle, &quitFlag); //send message to server
		}
	}
}
