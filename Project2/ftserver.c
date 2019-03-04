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

char* flip(char* ip)
{
	if(strcmp(ip, "128.193.36.41") == 0)
		return "flip3";
	
	return ip;
}



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
void recvMessage(int socketfd, char buffer[], int size)
{
	int charsRecv;
        memset(buffer, '\0', size); //Set buffer
        charsRecv = recv(socketfd, buffer, size, 0); // Read data from the socket
        if (charsRecv < 0) {
		fprintf(stderr, "CLIENT: ERROR reading from socket in recvMessage\n");
	}
	send(socketfd, "ACK", strlen("ACK"), 0); //send message
	
}

void sendMessage(int socketfd, char* message)
{
	char buffer[100];

	int charsSent = send(socketfd, message, strlen(message), 0); //send message
	if (charsSent < 0) perror("CLIENT: ERROR writing to socket");
	if (charsSent < strlen(message)) fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
	//Receive acknowledgemnt so messages don't arrive on single buffers at clent
        recv(socketfd, buffer, sizeof(buffer), 0); // wait for acknowledgement from server
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

//Source: https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
void sendDirectory(int socketfd)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG)
			{
				sendMessage(socketfd, dir->d_name);
			}
		}
	closedir(d);
	}
	sendMessage(socketfd, "***FIN");
}


int fileExists(int socketfd, char* filename)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	int found = 0;
	if (d) 
	{
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG && strcmp(dir->d_name, filename) == 0)
			{
				found = 1;
				printf("File found\n");
				sendMessage(socketfd, dir->d_name);
				break;
			}
		}
		closedir(d);
		if(found == 0)
		{
			printf("File not found\n");
			sendMessage(socketfd, "File not found");
		}
	}

	return found;
}


void handleRequest(int socketfd, char* client_ip)
{
	char command[10];
	char port[10];
	char filename[100];
	struct addrinfo* res;
	int sendfd;

	recvMessage(socketfd, port, sizeof(port));  //receive port number
//	fprintf(stdout, "Port %s\n", port); //print the message sent by server

	recvMessage(socketfd, command, sizeof(command)); //recieve command
	fprintf(stdout, "Command: %s\n", command); //print the message sent by server
	if (strcmp(command, "-g") == 0)
	{
		printf("Geeting filename\n");
		recvMessage(socketfd, filename, sizeof(filename));
		printf("Filename received\n");
		if (fileExists(socketfd, filename) == 0)
			return;
		
	}

	//Create socket and connect to client computer
	res = loadAddrinfo(client_ip, port);
	sleep(2);
	sendfd = createSocket(res);
	if(connect(sendfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Error connecting socket in handleRequest()");
		close(sendfd);
		return;
	}

	if (strcmp(command, "-l") == 0)
	{
		printf("List directory requested on port %s\n", port);
		printf("Sending directory contents to %s:%s\n", flip(client_ip), port);
		sendDirectory(sendfd);
	}


	//sendMessage(sendfd, "Test\n");

	close(sendfd);

}

int main(int argc, char* argv[])
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;
	int socketfd, new_fd;
	int numConnections = 5;
	char client_ip[INET6_ADDRSTRLEN];


	socketfd = startUp(argv[1], numConnections);

	while(1)
	{
		// now accept an incoming connection:
		new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), client_ip, sizeof client_ip);
		printf("Connection from %s\n", flip(client_ip));

		handleRequest(new_fd, client_ip);

		close(new_fd);
	}

	close(socketfd);
//	printf("Server closed\n");
	return 0;

}
