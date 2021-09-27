#include "header.h"

/* This function reads the inputs supplied by the command line, raises an error if any of them are invalid.
   Supplied params:
   argc, argv : (in) Command line argument count and strings
   sock : (out) A pointer to the location where the socket descriptor needs to be filled
   server : (out) A pointer to the location where the server structure needs to be filled
   filename : (out) Pointer to the location to where the file-name needs to be written at
   option : (out) GET or PUT, the type of operation.
   The return value is 0 on normal execution, -1 on error. */
int initialise(int argc, char** argv, int* sock, struct sockaddr_in* server, char* filename, enum option_t* option)
{
	struct hostent* host; /* hostent structure to store the host information which will be loaded on to the server param */

	if(argc != 5)
	{
		fprintf(stderr, "Usage: %s Server_IP Server_Port [GET/PUT] filename\n", argv[0]);
		return -1;
	}

	/* Get host IP by converting from dotted-decimal notation */
	host = gethostbyname(argv[1]);
	if(host == NULL)
	{
		fprintf(stderr, "Unknown IP: %s\n", argv[1]);
		return -1;
	}

	/* Create a UDP socket for communication with the server */
	*sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(*sock < 0)
	{
		fprintf(stderr, "Socket creation error\n");
		return -1;
	}

	/* Adding a timout option on the receiving blocking call for the socket. */
    struct timeval timeout = {0, 0};
    timeout.tv_sec = TIMEOUT_LEN;
    socklen_t opt_len = sizeof(timeout);
    if(setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, opt_len) < 0)
	{
		fprintf(stderr, "Socket option setting error.\n");
		return -1;
	}

	printf("Socket created.\n");

	/* Fill the server structure with appropriate params */
	server->sin_family = AF_INET;
	memcpy(&(*server).sin_addr.s_addr, host->h_addr, host->h_length);
	server->sin_port = htons(atoi(argv[2]));

	if(atoi(argv[2]) < 0 || atoi(argv[2]) > 65535)
	{
		fprintf(stderr, "Invalid port number.\n");
		return -1;
	}

	/* Extract option from argv */
	if(strcmp(argv[3], "GET") == 0)
	{
		*option = GET;
	}
	else if(strcmp(argv[3], "PUT") == 0)
	{
		*option = PUT;
	}
	else
	{
		fprintf(stderr, "Wrong option supplied, use GET or PUT\n");
		return -1;
	}

	/* Extract filename from argv */
	strcpy(filename, argv[4]);

	return 0;
}
