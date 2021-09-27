all: client 

client: 
	gcc -o client client.c init.c readPackets.c makePackets.c transfer.c

clean:
	rm client 
