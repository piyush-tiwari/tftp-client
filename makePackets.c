/* This file defines helper functions that make packets that are used in TFTP, based on RFC1350, self explanatory.
   All of them return the length of the packet formed. */

#include "header.h"

/*   2 bytes     string    1 byte     string   1 byte
    ------------------------------------------------
    | Opcode |  Filename  |   0  |    Mode    |   0  |
    ------------------------------------------------
               Figure 1: RRQ/WRQ packet
*/
int make_rrq(char* packet, char* filename, char* mode)
{
    int length = 2 + strlen(filename) + 1 + strlen(mode) + 1;

    *(opcode_t*)(packet) = htons(OP_RRQ);
    strcpy(packet + 2, filename);
    strcpy(packet + 2 + strlen(filename) + 1, mode);

    return length;
}

int make_wrq(char* packet, char* filename, char* mode)
{
    int length = 2 + strlen(filename) + 1 + strlen(mode) + 1;

    *(opcode_t*)(packet) = htons(OP_WRQ);
    strcpy(packet + 2, filename);
    strcpy(packet + 2 + strlen(filename) + 1, mode);

    return length;
}

/*     2 bytes     2 bytes      n bytes
       ----------------------------------
      | Opcode |   Block #  |   Data     |
       ----------------------------------
            Figure 2: DATA packet
*/
int make_data(char* packet, block_num_t block_num, char* data, int data_len)
{
    int length = 2 + 2 + data_len;

    *(opcode_t*)(packet) = htons(OP_DATA);
    *(block_num_t*)(packet + 2) = htons(block_num);
    memcpy(packet+4, data, data_len);

    return length;
}

/*       2 bytes     2 bytes
         ---------------------
        | Opcode |   Block #  |
         ---------------------
         Figure 3: ACK packet
*/
int make_ack(char* packet, block_num_t block_num)
{
    int length = 2 + 2;

    *(opcode_t*)(packet) = htons(OP_ACK);
    *(block_num_t*)(packet + 2) = htons(block_num);

    return length;
}

/*     2 bytes     2 bytes      string    1 byte
       -----------------------------------------
      | Opcode |  ErrorCode |   ErrMsg   |   0  |
       -----------------------------------------
                Figure 4: ERROR packet
*/
int make_err(char* packet, e_code_t e_code, char* e_message)
{
    int length = 2 + 2 + strlen(e_message);

    *(opcode_t*)(packet) = htons(OP_ERR);
    *(e_code_t*)(packet + 2) = htons(e_code);
    strcpy(packet + 4, e_message);

    return length;
}
