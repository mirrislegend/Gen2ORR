#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


int main(int argc, char *argv[])
{
	//relay entries
	typedef struct relay_entry {
		int position;
		struct sockaddr_in relay_addr;
		int port_number;
		int occupied;
	} relay_table[10];
	

	//channels
	typedef struct channel{
		char *channel_name;
		struct relay_entry subscribers[10];
	} channel_table[10];


	void setUpRelayTable(struct relay_entry []);
	void setUpChannelTable(struct channel[]);
	void setUpServerSocket(int, char**);



	//setting up
	setUpRelayTable(relay_table);
	setUpChannelTable(channel_table);
	setUpServerSocket(argc, argv);
	
	return 0;
}

//"relay entry" = "entry in relay table"
void setUpRelayTable(struct relay_entry *table[])
{
	int capacity = 10;
	int n;
	for(n=0; n<capacity; ++n)
	{
		table[n].position = n;
		table[n].port_number = 34000+n;
		table[n].occupied = 0;

		/*
		keep an eye on first parameter of the memset method below, the & is gone because table_entry is already an address
		*/
		memset(&table[n].relay_addr, 0, sizeof(table[n].relay_addr));
		table[n].relay_addr.sin_family = AF_INET;
		table[n].relay_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		table[n].relay_addr.sin_port = htons(table[n].port_number);
	}
}

void setUpChannelTable(struct channel *table[])
{
	
	//setting up the channel names
	table[0].channel_name = "A";
	table[1].channel_name = "B";
	table[2].channel_name = "C";
	table[3].channel_name = "D";
	table[4].channel_name = "E";
	table[5].channel_name = "F";
	table[6].channel_name = "G";
	table[7].channel_name = "H";
	table[8].channel_name = "I";
	table[9].channel_name = "J";
	//intializing subscriber relay entries
	const int capacity = 10;
	int i, j;
	for (i = 0; i < capacity; ++i)
	{
		for (j = 0; j < capacity; ++j)
		{	
			relay_entry * table_entry = &table[i].subscribers[j];
			table_entry->occupied = 0;
			table_entry->position = j;
			memset(&table_entry->relay_addr, 0, sizeof(table_entry->relay_addr));
		}
	}
}

//this method is not called in the code written in this file
/*
void subscribe_to_channel(char *chann_req, struct sockaddr_in *subscriber_addr)
{
	int capacity = 10;
	int n;
	for(n = 0; n<capacity; ++n){
		if(*channel_table[n].channel_name == *chann_req){
			int j = 0;
			while(channel_table[n].subscribers[j].occupied==1){
				++j;
				if(j==10){
					fprintf(stderr, "%s\n", "The requested channel is full.");
					exit(1);
				}
			}
			relay_entry * table_entry = &channel_table[n].subscribers[j];
			table_entry->occupied = 1;
			table_entry->relay_addr = *subscriber_addr;
			table_entry->port_number = ntohs(subscriber_addr->relay_addr.sin_port);
			break;

		}
	}
}
*/

//add to relay table
struct sockaddr_in add_to_relay(struct sockaddr_in *addr)
{
	int j = 0;
	while(relay_table[j].occupied==1){
		++j;
		if(j==10){
			fprintf(stderr, "%s\n", "The relay is full.");
			exit(1);
		}
	}
	relay_entry * table_entry = &relay_table[j];
	table_entry->occupied = 1;
	table_entry->relay_addr = *addr;
	table_entry->relay_addr.sin_port = table_entry->port_number;

	return table_entry->relay_addr;
}


void serve(int fd, struct sockaddr_in *addr)
{	
	//setting up new socket for relay
	int relay_socket;
	if (relay_socket = socket(AF_INET, SOCK_STREAM, 0)<0)
	{
		perror("socket");
		exit(1);
	}

	//adding client to relay table and obtaining new port for allocation
	struct sockaddr_in new_addr = add_to_relay(addr);
	int allocated_port = new_addr.port_number;
	printf("Client will now be given the new port number on which to connect: %d\n", allocated_port);

	//writing the new port number to the client while handling possible errors
	int tempnum = htonl(allocated_port);
	if(write(fd, &tempnum, sizeof(tempnum))==-1)
	{
		perror("write");
		exit(1);
	}

	//binding new address to the new socket
	if (bind(relay_socket, (struct sockaddr *)&new_addr, sizeof(new_addr)) < 0) 
	{
		perror("bind");
		exit(1);
	}

	//put new socket into listening mode
	if (listen(relay_socket, 5)==-1)
	{
		perror("listen");
		exit(1);
	}

	//accepting new connection to new socket
	while (1) {
		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		if(csock = accept(relay_socket,
			(struct sockaddr *)&client_addr, &client_len)==-1)
		{
			perror("accept");
			exit(1);
		}
		printf("Received connection from %s\n",		
			inet_ntoa(client_addr.sin_addr));

	//testing new socket connection by writing to its socket
	if(write(csock, &tempnum, sizeof(tempnum))==-1)
	{
		perror("write");
		exit(1);
	}

	/*
	By now, a new connection should have been established with the client using the new port number. This is where we ask the client for which channel they would like to broadcast on. Once we have this we then call subscribe_to_channel using this channel and the address stored in new_addr above.

	*/
}

void setUpServerSocket(int argc, char const *argv[])
{
	//declaring variables and methods for later use
	int lsock;

	//setting up our address
	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(atoi(argv[1]));

	//error-handling
	if (argc != 2)
	{
		fprintf(stderr, "%s\n", "Usage: server port");
		exit(1);
	}

	//creating our socket
	if ((lsock = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		perror("socket");
		exit(1);
	}

	//binding address to socket
	if(bind(lsock, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1)
	{
		perror("bind");
		exit(1);
	}

	//putting socket into listening mode
	if (listen(lsock, 5)==-1)
	{
		perror("listen");
		exit(1);
	}

	//accepting connections
	while(1){
		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		//setting up client socket and accepting pending connection
		csock = accept(lsock, (struct  sockaddr *)&client_addr, &client_len);

		//acknowledgement
		printf("Received connection from %s\n", inet_ntoa(client_addr.sin_addr));

		//forking
		switch(fork()){
			case -1:
				perror("fork");
				exit(1);
			case 0:
				serve(csock, &client_addr);
				break;
			default:
				close(csock);
				break;
		}
	}
}
