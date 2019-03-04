/****************************************************************************************************
#  Course Name: CS 372 Introduction to Computer Networks
#  Project 2
#  Program Name: ftclient.c
#  Programmer: Joshua Nutt
#  Last Modified: 3/4/2019
#  Description: This program is an FTP server.  It creates a listening socket, which FTP clients can
#  connect to to request a list of files or a transfer of a single file.  The server then creates a 
#  data connection to send the requested data through. The server can be terminated by Ctrl+C
#  Source: Beej's Guide to Network Programming Using Internet Sockets
*****************************************************************************************************/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/***********************************************************************
 * Description: Takes the IP address of a flip server and returns the
 * server name as a string
 * Parameters: The IP address of a flip server as a string
 * Returns: The anme of the flip server as a string
 * Pre-conditions: The IP address has been saved as a string
 * Post-conditions: The server name is outputted as a string
 * ********************************************************************/
char* flip(char* ip)
{
	if(strcmp(ip, "128.193.54.168") == 0)
		return "flip1";
	else if(strcmp(ip, "128.193.54.182") == 0)
		return "flip1";
	else if(strcmp(ip, "128.193.36.41") == 0)
		return "flip3";
	
	return ip; //return the numeric IP address by default
}



/***********************************************************************
 * Description: Returns a socket address, whhether it s IPv4 or IPv6
 * Parameters: Pointer sockaddr structure
 * Returns: The sockaddr address
 * Pre-conditions: The sockaddr pointer id pointing to valid structure
 * Post-conditions: The address is returned
 * Source: Beej's Guide to Network Programming Using Internet Sockets
 * ********************************************************************/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/***********************************************************************
 * Description: Receives message from client and send acknowledgement
 * back to client
 * Parameters: A socket to use to connect, the buffer the message will
 * be saved to, the size if the buffer
 * Pre-conditions: A socket connection has been established
 * Post-conditions: A message is saved in the buffer, and the
 * acknowledgment sent to the client.
 * ********************************************************************/
void recvMessage(int socketfd, char buffer[], int size)
{
	int charsRecv;
        memset(buffer, '\0', size); //Set buffer
        charsRecv = recv(socketfd, buffer, size, 0); // Read data from the socket
        if (charsRecv < 0) {
		fprintf(stderr, "CLIENT: ERROR reading from socket in recvMessage\n");
	}
	send(socketfd, "ACK", strlen("ACK"), 0); //send acknowledgement
	
}

/***********************************************************************
 * Description: Sends message to client and then waits for
 * acknowledgement
 * Parameters: A socket to use to connect, the buffer that holds the 
 * message
 * Pre-conditions: A socket connection has been established
 * Post-conditions: Message has been sent to client and acknowledgement
 * received
 * ********************************************************************/
void sendMessage(int socketfd, char* message)
{
	char buffer[100];

	int charsSent = send(socketfd, message, strlen(message), 0); //send message
	if (charsSent < 0) perror("CLIENT: ERROR writing to socket");
	if (charsSent < strlen(message)) fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
	//Receive acknowledgemnt so messages don't arrive on single buffers at client
        recv(socketfd, buffer, sizeof(buffer), 0); // wait for acknowledgement from server
}

/**********************************************************************************
 * Description: This function takes address information and adds it to the addrinfo
 * pointer
 * Parameters: The port number as a string
 * Returns: a pointer to a struct addrinfo with the address
 * Pre-conditions: the user has entered the port number into the commandline
 * Post-conditions: Address info is saved to the addrinfo struct
 * Source: Beej's Guide to Network Programming Using Internet Sockets
 * *******************************************************************************/
struct addrinfo* loadAddrinfo(char* address, char* port)
{
	int status;
	struct addrinfo hints;
	struct addrinfo* res;
	
	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_STREAM; //TCP Connections
	if(address == NULL)
		hints.ai_flags = AI_PASSIVE; //Us the localhost

	if((status = getaddrinfo(address, port, &hints, &res)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		exit(1);
	}
	
	return res;
}

/**********************************************************************************
 * Description: This function creates a socket
 * Parameters: A pointer to an addrinfo structure that contains info needed to
 * create socket
 * Returns: a scoket file descriptor as an int
 * Pre-conditions: The addrinfo pointer points to a valid structure
 * Post-conditions: The socket has been created
 * *******************************************************************************/
int createSocket(struct addrinfo *res)
{
	int socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(socketfd < 0){
		perror("Error creating socket");
		exit(1);
	}
	return socketfd;
}
/**********************************************************************************
 * Description: This function creates a socket, binds it, and sets it to listen
 * Parameters: The port number as a string, the number of maximum number of
 * connections that the socket will accept
 * Returns: a scoket file descriptor as an int
 * Pre-conditions: the user has entered the port number into the commandline and
 * set the maximumn number of incoming connects
 * Post-conditions: The socket is listening for connections
 * Source: Beej's Guide to Network Programming Using Internet Sockets
 * *******************************************************************************/
int startUp(char* port, int numConnections)
{
	int socketfd;
	struct addrinfo *res;
	
	//save the address information
	res = loadAddrinfo(NULL, port);

	//create socket
	socketfd = createSocket(res);

	//bind the socket
	if(bind(socketfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Error binding socket");
		close(socketfd);
		exit(1);
	}
	
	//set the socket to listen
	if (listen(socketfd, numConnections) < 0) {
		perror("Error listening with socket");
		close(socketfd);
		exit(1);
	}

	freeaddrinfo(res); // free the linked list

	return socketfd;

}

/**********************************************************************************
 * Description: This functions gets the list of files from a directory and 
 * send it to the client
 * Parameters: A socket file descriptor to send the directory on
 * Pre-conditions: The socket has been connected to the client
 * Post-conditions: The list of files have been sent to the client
 * Source: https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
 * *******************************************************************************/
void sendDirectory(int socketfd)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	if (d) { //if directory exits
		while ((dir = readdir(d)) != NULL) { //until reaching end of directory
			if (dir->d_type == DT_REG) //if this is a regular file
			{
				sendMessage(socketfd, dir->d_name); //send filename to client
			}
		}
	closedir(d);
	}
	sendMessage(socketfd, "***FIN"); //Let client know that is end of file list
}


/**********************************************************************************
 * Description: This functiion tests to see if a file exists in the directory
 * Parameters: A socket file descriptor to send the directory to send the response
 * on, the filename as a string
 * Returns: 1 if file exists, 0 if fil was not found
 * Pre-conditions: The socket has been connected to the client
 * Post-conditions: The list of files have been sent to the client
 * Source: Modified from
 * https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
 * *******************************************************************************/
int fileExists(int socketfd, char* filename)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	int found = 0;
	if (d)  //if directory exits
	{
		while ((dir = readdir(d)) != NULL) { //until reaching end of directory
			//If this is a regulat file and the name mathes the request file name
			if (dir->d_type == DT_REG && strcmp(dir->d_name, filename) == 0)
			{
				found = 1;
				printf("File found\n");
				sendMessage(socketfd, dir->d_name); //send acknowledgment
				break;
			}
		}
		closedir(d);
		if(found == 0) //if the file was not fount
		{
			printf("File not found\n");
			sendMessage(socketfd, "File not found"); //let the client know file was not found
		}
	}

	return found;
}

/**********************************************************************************
 * Description: This function sends a file over a socket
 * Parameters: A socket file descriptor to send the directory to send the response
 * on, the filename as a string
 * Pre-conditions: The socket has been connected to the client
 * Post-conditions: The file has been sent to the client
 * *******************************************************************************/
void fileSend(int socketfd, char* filename)
{
	int fileDesc = open(filename, O_RDONLY);
	if (fileDesc < 0)
	{
		fprintf(stderr, "Not able to open %s for reading\n", filename);
		return;
	}
	int fileSize = lseek(fileDesc, 0 , SEEK_END); //get number of characters in text file
	char buffer[fileSize]; //create buffer to hold all the chars from the file;
	int charsRead, charsSent; //stores the number of characters sent and received
    
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    
	lseek(fileDesc, 0, SEEK_SET); //reset offset to begining of file

	//Send the file size to the client so the client knows how many characters it will received
	char size[10];
	sprintf(size, "%d",fileSize);
	sendMessage(socketfd, size); 

	charsRead = read(fileDesc, buffer, fileSize); //read the text from the file
	charsSent = send(socketfd, buffer, charsRead, 0); // Write to the server
	if (charsSent < 0) perror("CLIENT: ERROR writing to socket");
	if (charsSent < charsRead) printf("CLIENT: WARNING: Not all data written to socket!\n");

	//Make sure all data was sent. Code from CS344 Lecture 4.2 by Benjamin Brewster
	int checkSend = -5;  // Bytes remaining in send buffer
	do
	{
		ioctl(socketfd, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
  	}while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0)  // Check if we actually stopped the loop because of an error
    		perror("ioctl error");

	send(socketfd, "***FIN", sizeof("***FIN"), 0); // Write to the server

	close(fileDesc); //close the file
	
	return;
}


/**********************************************************************************
 * Description: This function receives a request to send a file list or a file
 * to the client, opens a control connection, and then executes that request
 * Parameters: A socket file descriptor to receive the request on, the client's IP address
 * as a string
 * Pre-conditions: The socket has been connected to the client
 * Post-conditions: The directory or list of files has been sent to the client
 * Source: Beej's Guide to Network Programming Using Internet Sockets
 * *******************************************************************************/
void handleRequest(int socketfd, char* client_ip)
{
	char command[10];
	char port[10];
	char filename[100];
	struct addrinfo* res;
	int sendfd;

	//get the data port from the client
	recvMessage(socketfd, port, sizeof(port));  //receive port number
	//get the command from the client
	recvMessage(socketfd, command, sizeof(command)); //receive command
	//if the command was to get a file, also get the filename and test if file exists
	if (strcmp(command, "-g") == 0)
	{
		recvMessage(socketfd, filename, sizeof(filename));
		printf("File \"%s\" requested on port %s\n", filename, port);
		if (fileExists(socketfd, filename) == 0) //if the file was not found
			return;
		
	}

	//Create data transfer socket and connect to client computer
	res = loadAddrinfo(client_ip, port);
	sleep(1); //give client time to set up listening socket
	sendfd = createSocket(res); 
	if(connect(sendfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Error connecting socket in handleRequest().  Probably a bad port# from client");
		close(sendfd);
		return;
	}

	//if the client asked for a list of files
	if (strcmp(command, "-l") == 0)
	{
		printf("List directory requested on port %s\n", port);
		printf("Sending directory contents to %s:%s\n", flip(client_ip), port);
		sendDirectory(sendfd);
	}
	//If the client requested a file transfer
	else if (strcmp(command, "-g") == 0)
	{
	 	printf("Sending file\n");
		fileSend(sendfd, filename);
	 	printf("File sent\n");

	}
	
	close(sendfd); //close the data connection
}

/**********************************************************************************
 * Description: Main function of the program.
 * Parameters: The port to establish a listening socket
 * Pre-conditions: The port number is valid and available
 * Post-conditions: Requests have been handled and sockets have been closes
 * Source: Beej's Guide to Network Programming Using Internet Sockets
 * *******************************************************************************/
int main(int argc, char* argv[])
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;
	int socketfd, new_fd;
	int numConnections = 5;
	char client_ip[INET6_ADDRSTRLEN];

	//Create a socket to listen to incoming connections
	socketfd = startUp(argv[1], numConnections);

	while(1)
	{
		// now accept an incoming connection:
		new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
		//Get the clients IP address.  Source: Beej's Guide
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), client_ip, sizeof client_ip);
		//Print the client connection
		printf("Connection from %s\n", flip(client_ip));
		//Handle the request from the client
		handleRequest(new_fd, client_ip);
		//close the incoming connection
		close(new_fd);
	}
	//close the listening socket
	close(socketfd);

	return 0;
}
