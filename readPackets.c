/* Helper functions that extract specific portions of the packets supplied to them. */

#include "header.h"

/* extracts and returns the op-code of the packet supplied */
opcode_t extract_opcode(char* packet)
{
	return ntohs(*(opcode_t*)(packet));
}

/* extracts and returns the block-number on the packet supplied, use for ACK or DATA packets */
block_num_t extract_block_num(char* packet)
{
	return ntohs(*(block_num_t*)(packet + 2));
}

/* extracts and returns the error code on the supplied ERR packet */
e_code_t extract_e_code(char* packet)
{
	return ntohs(*(e_code_t*)(packet + 2));
}

/* extracts and returns the error message on the supplied ERR packet */
char* extract_e_message(char* packet, int pack_len)
{
	char* message = calloc(sizeof(char), pack_len - 4);
	strcpy(message, packet + 4);

	return message;
}

/* extracts the data on the supplied packet and writes it on the supplied buffer
   returns the amount of data read */
int extract_data(char* packet, char* data, int pack_len)
{
	int data_len = pack_len - 4;
	memcpy(data, packet+4, data_len);

	return data_len;
}
