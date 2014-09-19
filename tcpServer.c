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
	int channel_name;
	int channelport;
	struct sockaddr_in channel_addr;
	int channelsocket;
	char chanbuff[1024];
	int numsub;
	int subscriber[10]; //(file descriptors of client sockets)
} channel;


//takes in a table of channels. mutates table. returns nothing.
void setUpChannelTable(channel setTable[])
{

	//setting up the channel names
	setTable[0].channel_name = 1;
	setTable[1].channel_name = 2;
	setTable[2].channel_name = 3;
	setTable[3].channel_name = 4;
	setTable[4].channel_name = 5;
	setTable[5].channel_name = 6;
	setTable[6].channel_name = 7;
	setTable[7].channel_name = 8;
	setTable[8].channel_name = 9;
	setTable[9].channel_name = 10;

	//intializing channels
	for (int i = 0; i < 10; i++) //each channel has
	{
		setTable[i].channelport=34000+i; //a port number
//a buffer //change to dynamic inside malloc: "sizeof" //not needed if charbuff turned into an array
		setTable[i].chanbuff[0] = '\0';
		setTable[i].numsub=0;		//a count of the number of subscribers

		//a socket 	
		if ((setTable[i].channelsocket=socket(AF_INET, SOCK_STREAM,0)) < 0)
		{
			perror("socket");
			exit(1);
		}

		//with an address	
		memset(&(setTable[i].channel_addr), 0, sizeof(setTable[i].channel_addr));
		setTable[i].channel_addr.sin_family = AF_INET;
		setTable[i].channel_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		setTable[i].channel_addr.sin_port = htons(setTable[i].channelport);


		//attach 
		if (bind(setTable[i].channelsocket, (struct sockaddr *)&(setTable[i].channel_addr), sizeof(setTable[i].channel_addr)) < 0)
		{
			perror("bind");
			exit(1);
		}

		//put channel socket into listening mode
		if (listen(setTable[i].channelsocket, 5)==-1)
		{
			perror("listen");
			exit(1);
		}

		int capacity = 10;

		//initialize all subscribers to zero
		for (int j=0; j<capacity; j++)
		{
			setTable[i].subscriber[j]=0; //why doesn't this assignment stick?
		}
		
	}

}

//this method is not called in the code written in this file
//this will be used once the other issues are ironed out
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


int subscribe_to_channel(channel c, int clsock)
{
	
	printf("%s\n", "entered the subscribe_to_channel method");

	int x = htons(c.channelport);

	//send port number of channel to client
	if (write(clsock, &x, sizeof(x))<0)
	{
		perror("write");
		exit(1);
	}

	printf("%s\n", "wrote to client the socket corresponding to the client's desired channel");	


	printf("Socket ID of client when connected to rendezvous: %d \n", clsock);

	//accept client's connection to channel's socket
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	clsock=accept(c.channelsocket, (struct  sockaddr *)&client_addr, &client_len);

	printf("Socket ID of client when connected to channel: %d \n", clsock);
	
	printf("%s\n", "Testing connection between channel and client");

	char test[1024];
	if (read(clsock, test, sizeof(test))<0)
	{
		perror("write");
		exit(1);
	}
	printf("%s\n", test);
	printf("%s\n", "Connection from client to channel functions!");

	return clsock;

//neither of these loops worked. I had to move the functionality into the main method for it to work
//it is terrible coding practice to do that. but now it works. and all it cost me was my pride.
/*
	int n=0;
	while(1) //find an open slot
	{	
		printf("%d\n", c.subscriber[n]);
		if (c.subscriber[n]==0)
		{
			c.subscriber[n]=clsock;

			printf("%d \n", c.numsub);
			c.numsub=(c.numsub)+1;  //just subscribed a member, so increment the number of subscribers
			printf("%d \n", c.numsub);

			break;
		}
		else
		{
			n++;
		}
	}
*/

/*
	for (int n=0; n<10; n++)
	{
		
		if (c.subscriber[n]==0)
		{
			c.subscriber[n]=clsock;
			printf("Subscriber in %d slot\n", n);

			//printf("Number of subscribers before incrementing: %d \n", c.numsub);
			//c.numsub=(c.numsub)+1;  //just subscribed a member, so increment the number of subscribers
			//printf("Number of subscribers after incrementing: %d \n", c.numsub);

			break;
		}
	}
*/

	
}



/*
void leave_channel(int clsock)
{
	overwrite slot with zero
}
*/



void channelserve(channel c)
{
	//entire channel socket is already in channel table

	printf("%s \n", "you've reached the channelserve method! hooray!");
	printf("%s \n", "MAKE SURE TO KILL THE SERVER SIDE FIRST");
	

	int n=c.numsub;
	while(1)
	{	
		//printf("%s \n", "Start loop for constantly handling channel");
		//infinite copies of above print statement! hooray!

		for (int i=0; i<n; i++) //for each member
		{
			printf("Read from member %d \n", i);	
			int size;
			size=read(c.subscriber[i], c.chanbuff, strlen(c.chanbuff));
			
			if(size<0) //read from that member
			{
				perror("read");
				exit(1);
			}
			c.chanbuff[size]='\0';		

			printf("%s \n", c.chanbuff);
			printf("%s \n", "This print statement executes immediately after printing data from client");
			
			 
			/*
			if(strcmp(c.chanbuff,"") != 0) //if something is read in
			{
				for (int j=0; j<n; j++) //for each member
				{
					if (j!=i) //that is not the current member
					{
						printf("Write to member %d \n", j);
						if (write(c.subscriber[j], c.chanbuff, strlen(c.chanbuff))<0) //write to that member
						{
							perror ("read");
							exit(1);
						}
						
					}
				}
			}
			*/
			break;
		}
		break;

	}

}



//need a new experiment to check on waiting messages: client 1 sends a message and client 2 sends a message. if client 1's message gets read in and then written to everyone, what is on the connection with client 2? does reading from client 2 still yield the client 2 message? does client 2 still receive the client 1 message?
//that works!

int main(int argc, char const *argv[])
{
	//declaring variables and methods for later use
	int lsock;
	//channel table[];
	channel table[10];


	//setting up
	setUpChannelTable(table);
	

	//setting up our address
	struct sockaddr_in rendez_addr;
	memset(&rendez_addr, 0, sizeof(rendez_addr));

	rendez_addr.sin_family = AF_INET;
	rendez_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	rendez_addr.sin_port = htons(atoi(argv[1]));

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
	if(bind(lsock, (struct sockaddr *)&rendez_addr, sizeof(rendez_addr))==-1)
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

		//get channel-desired name
		char buff[256];
		int size;
		size=read(csock, buff, sizeof(buff));
		if (size<0)
		{
			perror("read");
			exit(1);
		}
		buff[size]='\0';
		
		printf("%s \n", buff);

		//find channel to go with desire-channel name
		int n=0;
		while (n<10)
		{
			if (table[n].channel_name==atoi(buff))
			{
				break;
			}
			else
			{
				n++;
			}
		}


		printf("%d \n", table[n].channelport);

		printf("%d \n", table[n].subscriber[0]);
		printf("%d \n", table[n].subscriber[1]);

		//printf("Channel %d with port number %d has %d members before subscription\n", table[n].channel_name, table[n].channelport, table[n].numsub);
			
		printf("Number of subscribers before incrementing: %d \n", table[n].numsub);

		int clsock = subscribe_to_channel(table[n], csock);

		for (int m=0; m<10; m++)
		{
			
			if (table[n].subscriber[m]==0)
			{
				table[n].subscriber[m]=clsock;
				printf("Subscriber in %d slot\n", m);

				break;
			}
		}

		
		table[n].numsub=(table[n].numsub)+1;  //just subscribed a member, so increment the number of subscribers
		printf("Number of subscribers after incrementing: %d \n", table[n].numsub);

		//printf("Channel has %d subscribers after subscription of newest member \n", table[n].numsub);

		

		int x = fork();
		switch (x)
		{
			case -1:
				perror("fork");
				exit(1);
			case 0:
				if(table[n].numsub==1)
				{
					printf("%s", "About to enter channelserve \n");
					channelserve(table[n]);
				}
				close(csock);
				break;
			default:
				close(csock);
				break;

		}
	
	}
	return 0;
}

