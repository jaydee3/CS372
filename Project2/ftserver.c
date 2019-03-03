#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

//get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/***********************************************************************
 * Description: Receives message from server and calls function to
 * send message
 * Parameters: A socket to use to connect, the user's handle
 * Pre-conditions: A socket conneection has been established
 * Post-conditions: A message has been received from the server, and the
 * user has had a chance to send a message
 * ********************************************************************/
void recMessage(int socketFD, char* handle)
{

        char buffer[500];

        memset(buffer, '\0', sizeof(buffer)); // Clear out
        int charsRecv = recv(socketFD, buffer, sizeof(buffer), 0); // Read data from the socket
        if (charsRecv < 0) fprintf(stderr, "CLIENT: ERROR reading from socket");

        if(!strcmp(buffer, "\\quit")) //if the server sent "\quit"
        {
                fprintf(stdout, "%s\n\n", "Session terminated by server");
 //               quitFlag = 1; //set flag to 1 so program knows to terminate
        }
        else //if the server did not "\quit"
        {
                fprintf(stdout, "%s\n", buffer); //print the message sent by server
   //             sendMessage(socketFD, handle, &quitFlag); //send message to server
        }
}


/**********************************************************************************
 * Description: This function takes address information and adds it to the addrinfo
 * pointer
 * Parameters: The port number as a string
 * Returns: a pointer to a struct addrinfo with the address
 * Pre-conditions: the user has entered the port number into the commandline
 * Post-conditions: The pointer is pointing the the addrinfo
 * Source: Beej's Guide to Network Programming Using Internet Sockets
 * *******************************************************************************/


//struct addrinfo* loadAddrinfo(char* port){
struct addrinfo* loadAddrinfo(char* address, char* port){
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
	socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(socketfd < 0){
		perror("Error creating socket");
		exit(1);
	}

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

int main(int argc, char* argv[])
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int socketfd, new_fd, sendfd;
	int numConnections = 5;
	char s[INET6_ADDRSTRLEN];
	char buffer[500];
	char portbuffer[500];
	int charsRecv;

	socketfd = startUp(argv[1], numConnections);

	// now accept an incoming connection:
	addr_size = sizeof their_addr;
	new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);


        memset(portbuffer, '\0', sizeof(portbuffer)); // Clear out
        charsRecv = recv(new_fd, portbuffer, sizeof(portbuffer), 0); // Read data from the socket
        if (charsRecv < 0) {
		fprintf(stderr, "CLIENT: ERROR reading from socket\n");
	}
        else 
		fprintf(stdout, "Port %s\n", portbuffer); //print the message sent by server

        memset(buffer, '\0', sizeof(buffer)); // Clear out
        charsRecv = recv(new_fd, buffer, sizeof(buffer), 0); // Read data from the socket
        if (charsRecv < 0) {
		fprintf(stderr, "CLIENT: ERROR reading from socket\n");
	}
        else 
		fprintf(stdout, "Command: %s\n", buffer); //print the message sent by server

	struct addrinfo* res;
	res = loadAddrinfo(s, portbuffer);
	//create socket
	sendfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sendfd < 0){
		perror("Error creating socket");
		exit(1);
	}
	if(connect(sendfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Error connecting socket");
	}

	close(sendfd);
	close(new_fd);

	printf("Server closed\n");
	return 0;

}
