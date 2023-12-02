#include <stdio.h>				// for printf() and fprintf()
#include <stdlib.h>				// for atoi() and exit()
#include <unistd.h> 			// for close(), read(), write() and getopt()
#include <string.h>				// for memset(), bzero(), and bcopy()
#include <fcntl.h>				// for fcntl(), F_GETFL, and F_SETFL
#include <pthread.h>			// for pthread_create(), pthread_join(), pthread_mutex_lock(), pthread_mutex_unlock(), pthread_cond_wait(), pthread_cond_signal(), and pthread_cond_broadcast()
#include <netinet/in.h>		// for struct sockaddr_in and htons()
#include <sys/socket.h>		// for socket(), bind(), listen(), accept(), and connect(), recv()
#include <sys/types.h>		// for struct sockaddr
#include <arpa/inet.h>		// for inet_ntoa()


void * receive_message(void * sockID)
{
	int clientSocket = *((int *) sockID);
	while(1)
	{
		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';
		printf("%s\n", data);
	}
}


void * send_message(void * sockID)
{
	int clientSocket = *((int *) sockID);
	while(1)
	{
		char data[1024];
		scanf("%s", data);
		send(clientSocket, data, 1024, 0);
	}
}


int main(int argc, char *argv[])
{
	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;

  if (argc != 3) 
	{
		printf("%s <dest_addr> <dest_port>", argv[0]);
		exit(1);
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddr.sin_port = htons(atoi(argv[2]));

	// for local testing
	// serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// serverAddr.sin_port = htons(8080);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("\nConnected to IRC Server!");
	printf("\nServer IP: %s", argv[1]);
	printf("\nServer Port: %s", argv[2]);
	printf("\nSocket ID: %d (local)\n\n", clientSocket);

	pthread_t receive_thread;
	pthread_create(&receive_thread, NULL, receive_message, (void *) &clientSocket );
	
	pthread_t send_thread;
	pthread_create(&send_thread, NULL, send_message, (void *) &clientSocket );

	// hold the main thread until the send and receive threads is finished
	pthread_join(receive_thread, NULL);
	pthread_join(send_thread, NULL);

}