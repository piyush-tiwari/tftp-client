/* This file defines the two prime functions of the client, to send and receive files from the server on the call of main() in client.c */

#include "header.h"

/* Requests and recieves a file from the server.
   The params are:
   sock     : (in) descriptor for the socket set up by initialise() in init.c, used to communicate with the server.
   server   : (in) sockaddr structure that contains the server IP and port number to be communicated with.
   filename : (in) filename that is to be read from the server, a file appended name recv_filename is created and written to.
   Returns 0 on normal termination and -1 on an error */
int recvfile(int sock, struct sockaddr_in server, char* filename)
{
	/* recv_buf holds the received packet, send_buf holds the packet to be sent or recently sent packet,
	   data_buf holds the data that is to be written on the file. */
	char recv_buf[BUF_SIZE], send_buf[BUF_SIZE], data_buf[BUF_SIZE];

	int recv_len = 0, send_len = 0, data_len = DATA_LIM; /* Length of the packet/data that each of the buffer is holding */
	block_num_t block_num = 1;                           /* block number of the next expected DATA packet */
	socklen_t server_len = sizeof(server);               /* length of the server structure */
	FILE *fptr;                                          /* File pointer used to access the write file */

	/* Create a RRQ packet to be sent to the server. We use the octet mode for transfer. */
	send_len = make_rrq(send_buf, filename, MODE_OCTET);

	/* Send the RRQ request to the server. */
	if(sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		fprintf(stderr, "Socket Error, failed to send RRQ packet.\n");
		return -1;
	}
	printf("Sending RRQ.\n");

	/* Open the file for writing the received contents. If the file doesn't exist, it is created,
	   if it already exists, it is over-written. We use the binary mode to transfer raw bytes. */
	char altfilename[MAX_FILENAME];
	strcpy(altfilename, "recv_");
	strcat(altfilename, filename);
	fptr = fopen(altfilename, "wb");
	if(fptr == NULL)
	{
		fprintf(stderr, "Unable to open file to write.\n");
		return -1;
	}
	printf("File %s opened for writing.\n", altfilename);

	/* An infinte loop that runs till the data_buf holds less than DATA_LIM bytes or error occurs. */
	do
	{
		/* Wait and receive the next packet, read it into read_buf */
		server_len = sizeof(server);
		recv_len = recvfrom(sock, recv_buf, BUF_SIZE, 0, (struct sockaddr*)&server, &server_len);

		/* Time-out. We terminate the transfer and send an error packet to the server. */
		if((recv_len < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
		{
			fprintf(stderr, "Time-out occured, sending an ERR packet to the server and closing connection. Transfer failed.\n");
			send_len = make_err(send_buf, 0, "Timeout, pre-mature termination.");
			sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server));
			fclose(fptr);
			return -1;
		}

		else if(recv_len < 0)
		{
			fprintf(stderr, "Socket error on receive.\n");
			fclose(fptr);
			return -1;
		}

		/* If an ERR packet is received from the server, close file and termiate transfer. */
		else if(extract_opcode(recv_buf) == OP_ERR)
		{
			fprintf(stderr, "Received ERR - %d: %s\n", extract_e_code(recv_buf), extract_e_message(recv_buf, recv_len));
			printf("Closing file, transfer failed.\n");
			fclose(fptr);
			return -1;
		}
		
		/* The DATA packet received has our next expected block number. */
		else if(extract_opcode(recv_buf) == OP_DATA && extract_block_num(recv_buf) == block_num)
		{
			printf("Received DATA, block=%d, sending ACK.\n", block_num);

			/* Extract data into data_buf */
			data_len = extract_data(recv_buf, data_buf, recv_len);

			/* Write from data_buf onto the file */
			fwrite(data_buf, sizeof(char), data_len, fptr);

			/* Form and send an ack packet */
			send_len = make_ack(send_buf, block_num);
			sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server));

			/* Increment the block number to indicate that this packet has been received. */
			block_num++;
		}

		/* The DATA packet is a duplicate one, send a duplicate ACK, but don't read the content */
		else if(extract_opcode(recv_buf) == OP_DATA && extract_block_num(recv_buf) < block_num)
		{
			send_len = make_ack(send_buf, extract_block_num(recv_buf));
			sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server));
			printf("Received DUPLICATE DATA, block=%d, sent duplicate ACK.\n", extract_block_num(recv_buf));
		}
	}
	while(data_len == DATA_LIM); /* If the data on data_buf is less than DATA_LIM, our transfer is completed. */

	printf("File %s received succesfully!\n", altfilename);
	fclose(fptr);
	return 0;
}

/* Sends a file to the server.
   The params are:
   sock     : (in) descriptor for the socket set up by initialise() in init.c, used to communicate with the server.
   server   : (in) sockaddr structure that contains the server IP and port number to be communicated with.
   filename : (in) filename that is to be written to the server, the mentioned file should exist in the directory of the client system.
   Returns 0 on normal termination and -1 on an error */
int sendfile(int sock, struct sockaddr_in server, char* filename)
{
	/* recv_buf holds the received packet, send_buf holds the packet to be sent or recently sent packet,
   	   data_buf holds the data has been read from the file */
	char recv_buf[BUF_SIZE], send_buf[BUF_SIZE], data_buf[BUF_SIZE];

	int recv_len = 0, send_len = 0, data_len = DATA_LIM; /* Length of the packet/data that each of the buffer is holding */
	block_num_t block_num = 0;                           /* block number of the last sent DATA packet */
	socklen_t server_len = sizeof(server);               /* length of the server structure */
	FILE *fptr;                                          /* File pointer used to access the write file */
	int num_resend = 0;                                  /* Keeps track of consecutive resends for a DATA packet. We terminate connection on MAX_RESEND */

	/* Open the file to be sent in binary read mode. Will throw error if file doesn't exist. */
	fptr = fopen(filename, "rb");
	if(fptr == NULL)
	{
		fprintf(stderr, "File doesn't exist on client.\n");
		return -1;
	}
	printf("File %s opened for reading.\n", filename);

	/* Make a WRQ packet. We use octet mode for transfer. */
	send_len = make_wrq(send_buf, filename, MODE_OCTET);

	/* Send the WRQ packet to the server. */
	if(sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		fprintf(stderr, "Socket Error, failed to send WRQ packet.\n");
		fclose(fptr);
		return -1;
	}
	printf("Sending WRQ.\n");

	/* Infinite loop that runs till the last DATA packet is sent, or an error occurs. */
	do
	{
		/* Receive the next packet from the server. We expect an ACK packet of block num = 0 on the first attempt,
		   and subsequent block numbers on proceeding transfers. */
		server_len = sizeof(server);
		recv_len = recvfrom(sock, recv_buf, BUF_SIZE, 0, (struct sockaddr*)&server, &server_len);

		/* Time-out, handle appropriately */
		if(recv_len < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
		{
			/* Resend limit exceeded, send ERR packet and close connection. */
			if(num_resend == MAX_RESEND)
			{
				printf("Timeout, resend limit exceeded, sending an ERR packet terminating transfer.\n");
				send_len = make_err(send_buf, 0, "DATA Resend limit exceeded.");
				sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server));
				fclose(fptr);
				return -1;
			}

			/* Else, resend the data packet on the send_buf */
			printf("Timeout, resending the last sent packet.\n");
			num_resend++;
			sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server));
		}

		/* If ERR is received, terminate transfer and close file. */
		else if(extract_opcode(recv_buf) == OP_ERR)
		{
			fprintf(stderr, "Received ERR - %d: %s\n.", extract_e_code(recv_buf), extract_e_message(recv_buf, recv_len));
			printf("Closing file, transfer failed.\n");
			fclose(fptr);
			return -1;
		}

		/* If the received ACK is for the expected block number, reset resend counter and send next data packet */
		else if(extract_opcode(recv_buf) == OP_ACK && extract_block_num(recv_buf) == block_num)
		{
			if(data_len < DATA_LIM) /*Less than DATA_LIM on the data_buf indicates a successful, completed transfer. */
			{
				printf("Received ACK for block=%d, transfer complete.\n", block_num);
				break;
			}

			if(block_num == 0)
				printf("Received ACK for WRQ, initiating DATA transfer.\n");
			else
				printf("Received ACK for block=%d, sending DATA for next block.\n", block_num);

			/* Increment the block_num to the next applicable block */
			block_num++;

			/* Reset the resend counter */
			num_resend = 0;

			/* Read the next DATA_LIM bytes from the file. If the bytes left are fewer, it is reflected in data_len */
			data_len = fread(data_buf, sizeof(char), DATA_LIM, fptr);

			/* Make a DATA packet on send_buf and send it to the server. */
			send_len = make_data(send_buf, block_num, data_buf, data_len);
			sendto(sock, send_buf, send_len, 0, (struct sockaddr*)&server, sizeof(server));
		}
	}
	while(1);

	fclose(fptr);
	printf("File %s sent succesfully!\n", filename);

	return 0;
}
