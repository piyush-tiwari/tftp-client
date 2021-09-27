/* This is a common header for the entire client, and cotains all the pre-processing definitions, headers and function declarations that are used */

#ifndef TFTP_HEADER_
#define TFTP_HEADER_

/* Library files used in the project */
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE       600         /* Size used for buffers, any number greater than DATA_LIM will do */
#define DATA_LIM       512         /* Max size of content in bytes on the DATA packet */
#define MODE_OCTET     "octet"     /* The client uses the octet mode for file transfer */
#define MODE_NETASCII  "netascii"  /* netascii mode is not used by this client, and it isn't mandatory as client will choose the mode of transfer */
#define MODE_MAIL      "mail"      /* Obsolete mode that shouldn't be used anymore */
#define TIMEOUT_LEN    5           /* Seconds of time before recvfrom times out */
#define MAX_RESEND     1           /* Number of times a DATA packet will be sent before close of transfer */
#define MAX_FILENAME   100         /* Max Filename size allowed. */

/* typedef aliases for 2-byte numbers used in packets */
typedef unsigned short opcode_t;
typedef unsigned short block_num_t;
typedef unsigned short e_code_t;
/* Options available for the client */
enum option_t {GET, PUT};

/* TFTP op-codes */
#define OP_RRQ      1   
#define OP_WRQ      2
#define OP_DATA     3
#define OP_ACK      4
#define OP_ERR      5

/* Functions in makePackets.c
   All of them receive a char buffer pointer onto which they write the created packets using the arguments supplied.
   They return the number of bytes written (length of packets) */
int make_rrq(char* packet, char* filename, char* mode);
int make_wrq(char* packet, char* filename, char* mode);
int make_data(char* packet, block_num_t block_num, char* data, int data_len);
int make_ack(char* packet, block_num_t block_num);
int make_err(char* packet, e_code_t e_code, char* e_message);

/* Functions in readPackets.c
   These are functions to read specific portions of the packet supplied to them as a buffer.
   extract_opcode, extract_block_num, extract_e_code and extract_e_message return the extracted content.
   extract_data returns the length (in bytes) of data written from the packet onto the supplied data buffer. */
opcode_t extract_opcode(char* packet);
block_num_t extract_block_num(char* packet);
e_code_t extract_e_code(char* packet);
char* extract_e_message(char* packet, int pack_len);
int extract_data(char* packet, char* data, int pack_len);

/* The function in init.c processes the command line input, sets up the socket, filename and option params.
   Returns 0 on successful completion, else -1 */
int initialise(int argc, char** argv, int* sock, struct sockaddr_in* server, char* filename, enum option_t* option);

/* The functions in transfer.c handle the actual file transfer when called by the main function in client.c
   Return 0 on successful completion, else -1 */
int recvfile(int sock, struct sockaddr_in server, char* filename);
int sendfile(int sock, struct sockaddr_in server, char* filename);

#endif
