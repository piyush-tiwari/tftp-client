/* Entry and exit point for the client, majorly delegates its tasks to other functions defined in the supporting files. */

#include "header.h"

int main(int argc, char* argv[])
{
	int sock;                      /* Socket fd */
	struct sockaddr_in server;     /* sockaddr_in strcuture that stores the server information */
	char filename[MAX_FILENAME];   /* filename to be received/sent */
	enum option_t option;          /* option that specifies the operation, GET or PUT */

	/* Call initialise in init.c to read the inputs and set up the socket and sockaddr structure, exit if error is encountered */
	if(initialise(argc, argv, &sock, &server, filename, &option) < 0)
		exit(1);

	/* On GET option, call the recvfile function to send the file to the server */
	if(option == GET)
	{
		if(recvfile(sock, server, filename) < 0)
			exit(1);
	}
	
	/* On GET option, call the recvfile function to send the file to the server */
	if(option == PUT)
	{
		if(sendfile(sock, server, filename) < 0)
			exit(1);
	}

	return 0;
}
