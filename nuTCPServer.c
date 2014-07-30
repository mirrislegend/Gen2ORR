#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	//setting up relay table
	struct relay_entries {
		int position;
		sockaddr_in relay_addr;
		int port_number;
		int occupied;
	}relayTable[10];
	setUpRelayTable(relayTable);

	//setting up channel table
	struct channels {
		string channel_name;
		relay_entries subscribers[];
	}channelTable[10];
	setUpChannelTable(channelTable);

	setUpSocket(argc, *argv[])
	return 0;
}

void setUpSocket(int argc, char const *argv[])
{
	//declaring variables and methods for latter use
	int lsock;
	void serve(int);

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
				serve(csock);
				break;
			default:
				close(csock);
				break;
		}
	}
}

void serve(int fd)
{	
	//the code below is just there, instead have some code here that puts things into the relay table and subscribes to channels
	char buf[1024];
	int count;
	while((count = read(fd, buf, 1024))>0){
		if (write(1, buf, count)==-1)
		{
			perror("write");
			exit(1);
		}
	}
	if (count == -1)
	{
		perror("read");
		exit(1);
	}
	printf("connection terminated\n");
}

void setUpRelayTable(relay_entries reTab[])
{
	int capacity = sizeof(reTab)/sizeof(reTab[0]);
	for(int n=0; n<capacity; ++n)
	{
		relay_entries * table_entry = &reTab[n];
		*table_entry.position = n;
		*table_entry.port_number = 34000+n;
		*table_entry.occupied = 0;

		/*
		keep an eye on first parameter of the memset method below, the & is gone because table_entry is already an adress
		*/
		memset(table_entry.relay_addr, 0, sizeof(*table_entry.relay_addr));
		*table_entry.relay_addr.sin_family = AF_INET;
		*table_entry.relay_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		*table_entry.relay_addr.sin_port = htons(*table_entry.port_number);
	}
}

void setUpChannelTable(channels chan[])
{
	//setting up the channel names
	chan[0].channel_name = "A";
	chan[1].channel_name = "B";
	chan[2].channel_name = "C";
	chan[3].channel_name = "D";
	chan[4].channel_name = "E";
	chan[5].channel_name = "F";
	chan[6].channel_name = "G";
	chan[7].channel_name = "H";
	chan[8].channel_name = "I";
	chan[9].channel_name = "J";
	//intializing subscriber relay entries
	int capacity = sizeof(chan)/sizeof(chan[0]);
	for (int i = 0; i < capacity; ++i){
		for (int j = 0; j < 10; ++j){	
			relay_entries * table_entry = &chan[i].subscribers[j];
			*table_entry.occupied = 0;
			*table_entry.position = j;
			memset(table_entry.relay_addr, 0, sizeof(*table_entry.relay_addr));
		}
	}
}

/*
Below is very rough.
1. We need to handle our errors better. What happens when the requested channel is full? Do forward the subscriber to another channel?
2. Secondly, we have issues with addresses. Is the address of a subscriber in the channel table meant to be that client's original address or the address given to them in the relay table?
3. Does this all mean that clients have to be written whole new addresses? I don't see how this is possible
*/
void subscribe_to_channel(string chann_req, sockaddr_in subscriber_addr)
{
	int capacity = sizeof(channelTable)/sizeof(channelTable[0]);
	for(int n=0; n<capacity; ++n){
		if(channelTable[n].channel_name==chann_req){
			int j = 0;
			while(channelTable[n].subscribers[j].occupied==1){
				++j;
				if(j==10){
					fprintf(stderr, "%s\n", "The requested channel is full.");
					exit(1);
				}
			}
			relay_entries * table_entry = &channelTable[n].subscribers[j];
			*table_entry.occupied = 1;
			*table_entry.relay_addr = subscriber_addr;
			*table_entry.port_number = ntohs(subscriber_addr.relay_addr.sin_port);
		}
	}
}
