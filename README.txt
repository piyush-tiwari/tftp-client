Attached is the Networks Lab (CS342), Assignment 3 submission by group 44
1. Tiwari Piyush Shailendra, 190123068
2. Ayush Raj               , 190123015
3. Manas Singh Jyoti       , 190123034
4. Vishnu Swaroop Rai      , 190123063

This is a TFTP client implemented in C based on the RFC1350 specification.

To compile the file, use the command "make" or "make client" to create the executable client file.
Use "make clean" to remove the compiled file.

Alternatively, type the following command:
gcc -o client client.c init.c transfer.c readPackets.c makePackets.c

The syntax to use the client is as follows:
./client <Server_IP> <Server_Port> <Option (GET or PUT)> <filename>

As an example:
./client 127.0.0.1 7000 GET sample.txt
./client 127.0.0.1 7000 PUT sample.txt


We recommend using a local tftp server for testing, for example, 'tftpd-hpa'.
Since TFTP is rarely used over the internet, firewalls block most of its traffic and a server rarely exists due to security issues.
A sample.txt file is attached to be used to test the client.
The transfer limit is 32 MB (This follows from the block number being only a 2 byte number, and each packet has 512 B), but the limit can be infinite if the responding server uses a roll-over to 0 for block number.

An incomplete transfer for RRQ results in an incomplete file.
The file received on a GET is named as recv_filename
For example, when requesting for sample.txt, the received file will be recv_sample.txt