#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


//channels
typedef struct{
	int channel_name;
	int channelport;
	struct sockaddr_in channel_addr;
	int channelsocket;
	char chanbuff[128]; //this is where channels will store messages during handoffs between clients
	int numsub;
	int subscriber[10]; //file descriptors of client sockets
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
		setTable[i].chanbuff[0] = '\0'; //a buffer
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


		//bind address and socket 
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
			setTable[i].subscriber[j]=0;
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

//this is called by the main method when it is time to subscribe a client to it's desired channel
int subscribe_to_channel(channel c, int clsock)
{
	
	printf("%s\n", "Entered the subscribe_to_channel method");

	//send port number of channel to client
	printf("%s\n", "Write, to client, the socket corresponding to the client's desired channel");
	int x = htons(c.channelport);
	//write for r/w pair #2
	if (write(clsock, &x, sizeof(x))<0)
	{
		perror("write");
		exit(1);
	}

	printf("Socket ID of client when connected to rendezvous: %d \n", clsock);

	close(clsock);

	//accept client's connection to channel's socket
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	clsock=accept(c.channelsocket, (struct  sockaddr *)&client_addr, &client_len);

	printf("Socket ID of client when connected to channel: %d \n", clsock);
	printf("These two socket IDs, and the corresponding pair on the client side, \n");
	printf("     will be the same or increment by one \n");
	printf("     depending on where the close statement is placed in the client side code \n");
	
	printf("%s\n", "Testing connection between channel and client");

	char subscribetestbuff[128];
	int size2;
	//read of r/w #3
	size2 = read(clsock, subscribetestbuff, sizeof(subscribetestbuff));
	if (size2<0)
	{
		perror("write");
		exit(1);
	}
	//subscribetestbuff[size]='\0';
	printf("%s \n", subscribetestbuff);


	return clsock;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//This code was inside the subscribe_to_channel method but neither loop of them worked. 
//I had to move the functionality into the main method for it to work
//It is terrible coding practice to do that. but now it works. and all it cost me was my integrity.
//Hopefully we can put this functionality back in subscribe_to_channel eventually
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	

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
	printf("%s \n\n", "MAKE SURE TO KILL THE SERVER SIDE FIRST");
	printf("%s \n", "Attempting to read from client as soon as channelserve is entered");

	char servetestbuff[128];
	int size1;
	size1=read(c.subscriber[0], servetestbuff, sizeof(servetestbuff)); //read from that subscriber
			
	if(size1<0) //error handling
	{
		perror("read");
		exit(1);
	}

	printf("Characters: %d. Message: %s \n\n", size1, servetestbuff);

//this is the code for continual discussion between client and server. obviously, this stuff is on hold for now
/*
	int n=c.numsub;
	while(1)
	{	
		//printf("%s \n", "Start loop for constantly handling channel");
		//infinite copies of above print statement! hooray!

		char servetestbuff[1024];

		for (int i=0; i<n; i++) //for each subscriber
		{

			fflush(stdout);
			
			printf("Read from member number %d on channel with fd number %d\n", i, c.subscriber[i]);	
			
			int size;
			size=read(c.subscriber[i], servetestbuff, sizeof(servetestbuff)); //read from that subscriber
			
			if(size<0) //error handling
			{
				perror("read");
				exit(1);
			}
			
			fflush(stdout);

			
			//servetestbuff[size]='\0';	//null terminator

			//c.chanbuff="fake input";

			//strcpy(servetestbuff, "fake input");	

			//printf("%s \n", servetestbuff);

			printf("%d characters were read in \n", size1);

			printf("%s \n", "This print statement executes immediately after printing data from client");
*/			
			
			//this is where the server writes out to the clients the data that was read in
			//i want to get the reading in part working first before i start writing out
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
/*			//sleep(3);
			break;
		}
		//sleep(3);
		break;


	}*/

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
		//program waits at accept. this means that the primary process of the program halts here, until a client tries to connect to the rendevous socket
		csock = accept(lsock, (struct  sockaddr *)&client_addr, &client_len);

		//acknowledgement
		printf("Received connection from %s. Waiting to receive channel.\n", inet_ntoa(client_addr.sin_addr));

		//get name from client of channel client desires
		char tempbuff1[128];
		int size3;

		//Read for read-write pair number 1
		size3=read(csock, tempbuff1, sizeof(tempbuff1));
		if (size3<0)
		{
			perror("read");
			exit(1);
		}
		
		//output to user for r/w #1
		printf("Client wants to be on channel: %d \n", atoi(tempbuff1));

		//find channel to go with desire-channel name
		int n=0;
		while (n<10)
		{
			if (table[n].channel_name==atoi(tempbuff1))
			{
				break;
			}
			else
			{
				n++;
			}
		}

		//more output to user for r/w #1
		printf("The port number of the desired channel is: %d \n", table[n].channelport);

			
		printf("Number of subscribers on that channel before latest client subscribes: %d \n", table[n].numsub);

		//call the subscribe method
		int clsock = subscribe_to_channel(table[n], csock); //contains write of r/w pair #2

		for (int m=0; m<10; m++)
		{
			
			if (table[n].subscriber[m]==0)
			{
				table[n].subscriber[m]=clsock;
				printf("Subscriber in %d slot of channel \n", m);

				break;
			}
		}
		
		table[n].numsub=(table[n].numsub)+1;  //just subscribed a member, so increment the number of subscribers

		printf("Number of subscribers after subscription of latest client: %d \n\n", table[n].numsub);

		
		//read/write pair #4
		//printf("Attempt to read BEFORE fork\n");
		char tempbuff2[128];
		int size4;
		size4=read(clsock, tempbuff2, sizeof(tempbuff2));
		printf("Attempt to read BEFORE fork:  "ANSI_COLOR_YELLOW "Characters: %d. Message: %s \n" ANSI_COLOR_RESET "\n", size4, tempbuff2);
		
		//fork off a child process
		int x = fork();
		switch (x)
		{
			case -1: //error
				perror("fork");
				exit(1);
			case 0: //child
				if(table[n].numsub==1) //fork off a child process ONLY when the client that just subscribed is the ONLY subscriber in its channel. This child process will still be running when new clients are subscribed to the channel and no new process is necessary. (this concept will need eventual updating, when subscribers can leave channels)
				{
					printf( ANSI_COLOR_RED "YOU ARE IN CHILD fork" ANSI_COLOR_RESET " for Chan %d\n", atoi(tempbuff1));
					int size;
					char testtest[128];
					size=read(table[0].subscriber[0], testtest, sizeof(testtest));
					printf("Characters: %d. Message: %s \n\n", size, testtest);
					printf("About to enter channelserve on channel %d \n", n+1);
					channelserve(table[n]);
				}
				close(csock);
				break;
			default: //parent
				close(csock);
				break;

		}
	
	}
	return 0;
}

