#include <stdio.h>				// for printf(), fprintf(), perror() and stderr
#include <stdlib.h>				// for atoi(), exit() and EXIT_FAILURE
#include <unistd.h>				// for close(), read(), write() and getopt()
#include <string.h>				// for memset(), bzero(), and bcopy()
#include <fcntl.h>				// for fcntl(), F_GETFL, and F_SETFL
#include <pthread.h>			// for pthread_create(), pthread_join(), pthread_mutex_lock(), pthread_mutex_unlock(), pthread_cond_wait(), pthread_cond_signal(), and pthread_cond_broadcast()
#include <netinet/in.h>		// for struct sockaddr_in and htons()
#include <sys/socket.h>		// for socket(), bind(), listen(), accept(), and connect()
#include <sys/types.h>		// for struct sockaddr and socklen_t
#include <arpa/inet.h>		// for inet_ntoa() and inet_aton()

int clientCount = 0; // number of clients connected

// mutex and condition variable for synchronization
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


// struct to hold client information
struct client
{
	int index;					            // index of client
	int sockID;					          	// socket ID of client	
	struct sockaddr_in clientAddr;  // client address
	int len;												// length of client address
};
struct client Client[1024];		// array of clients for each connection
pthread_t thread[1024];				// array of threads for each client


// function to handle client connections
void *handle_user_connection(void *ClientDetail)
{
	// get client information from ClientDetail struct and print it
	struct client *clientDetail = (struct client *)ClientDetail;
	int index = clientDetail->index;
	int clientSocket = clientDetail->sockID;
	printf("[SERVER]: Client %d connected at socket %d.\n", index, clientSocket);

	// send welcome message to client
	char welcome[1024];
	snprintf(welcome, 1024, "Welcome to the IRC server, client %d at socket %d.\n", index, clientSocket);
	send(clientSocket, welcome, 1024, 0);

	// receive data from client and send it to all other clients
	while (1)
	{
		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';
		char output[1024];

		// if there is data, send it to all clients except the sender
		if (read > 0)
		{
			for (int i = 0; i < clientCount; i++)
			{
				if (Client[i].sockID != clientSocket) 
				{
					snprintf(output, 1024, "[CLIENT %d]: %s", index, data);
					send(Client[i].sockID, output, 1024, 0);
				}
			}
		}
	}
	// close(clientSocket);
	return NULL; // return NULL pointer to terminate thread
}


int main(int argc, char *argv[])
{
	// Create socket for server to listen on
	int serverSocket = socket(PF_INET, SOCK_STREAM, 0); // socket descriptor for server
	struct sockaddr_in serverAddr; // server address

	// Check for correct number of arguments and print usage if incorrect
  if (argc != 3) 
	{
		printf("%s <listen_addr> <listen_port>", argv[0]);
		exit(1); // exit with error code 1
	}

	// Set up server address and port number for binding
	serverAddr.sin_family = AF_INET; // IPv4 address family
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]); // IP address
	serverAddr.sin_port = htons(atoi(argv[2]));	// Port number

	// DEBUG
	// serverAddr.sin_addr.s_addr = htons(INADDR_ANY);
	// serverAddr.sin_port = htons(8080);

	// Bind socket to port and IP address of server
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{ return 0; }

	// Listen for connections and specify max
	if (listen(serverSocket, 4) == -1)
	{ return 0; }

	printf("\nServer is running...");
	printf("\nIP: %s", argv[1]);
	printf("\nPort: %s\n\n", argv[2]);

	while (1)  // Accept connections
	{
		// Accept connection and create new socket for client to communicate
		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr *)&Client[clientCount].clientAddr, (socklen_t *)&Client[clientCount].len);
		Client[clientCount].index = clientCount;  // Assign client index
		pthread_create(&thread[clientCount], NULL, handle_user_connection, (void *)&Client[clientCount]);
		clientCount++; // Increment client count
	}

	// Wait for all threads to finish
	for (int i = 0; i < clientCount; i++)
	{ pthread_join(thread[i], NULL); }

}