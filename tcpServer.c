#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


//channels
typedef struct{
	char *channel_name;
	int channelport;
	//char *chanbuff;
	int numsub;
	int subscribers[10]; (file descriptors of sockets)
} channel;

channel channel_table[10];


void setUpChannelTable(channel table[])
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

	//intializing channels
	int i;
	for (i = 0; i < capacity; ++i)
	{
		table[i].channelport=3400+i;
		table[i].chanbuff="";
		table[i].numsub=0;
		
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


/***********************/
/*
void channelserve(channel)
{
	//entire channel socket is already in channel table

	//putting socket into listening mode
	if (listen(channel.socket, 5)==-1)
	{
		perror("listen");
		exit(1);
	}
	
	int n;
	char buff="";
	while(1)
	{	
		
		int clsock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		if((clsock = accept(relay_socket, (struct sockaddr *)&client_addr, &client_len))==-1)
		{
			perror("accept");
			exit(1);
		}

		printf("Received connection from %s.\n", inet_ntoa(client_addr.sin_addr));

		n=add_to_channel(clsock); //return number of members in channel

		for(int i; i<n; i++) //for each member
		{
			read(channel.member[i], buff, strlen(buff); //read from that member

			if (buff != "") //if something is read in then
			{
				chanbuff=buff;
			}
		}

	}

}

*/

//need a new experiment to check on waiting messages: client 1 sends a message and client 2 sends a message. if client 1's message gets read in and then written to everyone, what is on the connection with client 2? does reading from client 2 still yield the client 2 message? does client 2 still receive the client 1 message?


int main(int argc, char const *argv[])
{

	
	void setUpRelayTable(relay_entry[]);
	void setUpChannelTable(channel[]);
	

	//setting up
	setUpRelayTable(relay_table);
	setUpChannelTable(channel_table);


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
	
	printf("Please initialize client now \n");

	//accepting connection
	while(1)
	{
		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		//setting up client socket and accepting pending connection
		csock = accept(lsock, (struct  sockaddr *)&client_addr, &client_len);

		//acknowledgement
		printf("Received connection from %s. Waiting to receive channel.\n", inet_ntoa(client_addr.sin_addr));
/*
		//get channel-desired name
		char buff[2];
		if (read(csock, buff, sizeof(buff))==-1)
		{
			perror("read");
			exit(1);
		}
*/
		//channel table needs "# occupants" field

		//int n
		//while n<10
			//if table[n].channel_name==buff
				//break
			//else
				//n++
		
		//if table[n].occupants==zero
			//write port number of channel to client
			//fork
				//channelserve(channel)
				//close connection
		//else
			//write port number of channel to client
			//close connection
		

		//forking
		switch(fork())
		{
			case -1:
				perror("fork");
				exit(1);
			case 0:
				serve(csock, &client_addr);
				//closing the connection to the old socket
				if (close(csock)==-1)
				{
					perror("close");
					exit(1);
				}
				if (relayflag == 1)
				{
					close(lsock);
				}
				break;
			default:
				return 0;
				close(csock);
				break;
		}
		
	}
	return 0;
}

