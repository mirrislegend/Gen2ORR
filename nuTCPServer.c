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
		sockaddr_in client_addr;
		int port_number;
		int occupied;
	}relayTable[10];
	setUpRelayTable(relayTable);

	//setting up channel table
	struct channels {
		string channel_name;
		relay_entries subscribers[];
	}channelTable[10];

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
	for(int n=0; n<10; n++)
	{
		reTab[n].position = n;
		reTab[n].port_number = 34000+n;
		reTab[n].occupied = 0;		
	}
}

//getting into the channel table is called 'subscribing'
