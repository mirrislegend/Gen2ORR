//server file after all the lessons learned from tcpServer_test

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>  

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

int subscribe_to_channel(channel c, int clsock);
void channelserve(channel c);

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
		setTable[i].chanbuff[0] = '\0';  //a buffer
		setTable[i].numsub=0;		 //a count of the number of subscribers

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



	








//need a new experiment to check on waiting messages: client 1 sends a message and client 2 sends a message. if client 1's message gets read in and then written to everyone, what is on the connection with client 2? does reading from client 2 still yield the client 2 message? does client 2 still receive the client 1 message?
//that works!

int main(int argc, char const *argv[]){

	//error-handling
	if (argc != 2){
		fprintf(stderr, "%s\n", "Usage: server port");
		exit(1);
	}

	// check that arg is an integer
   	if (atoi(argv[1])==0){
		fprintf(stderr, "%s\n", "Usage: port must be an integer");
		exit(1);
	}

	//declaring variables and methods for later use
	int lsock;
	channel table[10];

	//setting up
	setUpChannelTable(table);

	//print contents of table
	//printChannelTable(table);
	
	//setting up our address
	struct sockaddr_in rendez_addr;
	memset(&rendez_addr, 0, sizeof(rendez_addr));

	rendez_addr.sin_family = AF_INET;
	rendez_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	rendez_addr.sin_port = htons(atoi(argv[1]));

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
	
	printf("Please initialize client now \n\n");

	//this loop represents the rendezvous constantly receiving incoming clients, making channels when possible, and handing those clients to channels 
	while(1)
	{
		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		//setting up client socket and accepting pending connection
		//program waits at accept. this means that the primary process of the program halts here, until a client tries to connect to the rendevous socket
		csock = accept(lsock, (struct  sockaddr *)&client_addr, &client_len);

		//acknowledgement
		printf("Received connection from %s. Waiting to receive channel.\n\n", inet_ntoa(client_addr.sin_addr));

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

			
		printf("Number of subscribers on that channel before latest client subscribes: %d \n\n", table[n].numsub);

	
		//call the subscribe method
		int clsock = subscribe_to_channel(table[n], csock); //contains write of r/w pair #2
		printf(ANSI_COLOR_GREEN"%s\n\n" ANSI_COLOR_RESET, "Exited the subscribe_to_channel method");

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//from here down to next big marker made of tilde's should be in the subscribe to channel method. read the notes on the subscribe to channel method for a better explanation
		for (int m=0; m<10; m++)
		{
			
			
			if (table[n].subscriber[m]==0)
			{
				table[n].subscriber[m]=clsock;

				printf("Channel %d, subscriber #%d has fd=%d\n", n+1, m+1, table[n].subscriber[m]);

				break;
			}
			else
			{
				printf("Channel %d, subscriber #%d has fd=%d\n", n+1, m+1, table[n].subscriber[m]);
			}
			

		}
		
		table[n].numsub=(table[n].numsub)+1;  //just subscribed a member, so increment the number of subscribers

		printf("Number of subscribers after subscription of latest client: %d \n\n", table[n].numsub);

		
		//GetSocketOptions(clsock);
		//read/write pair #4
		char tempbuff2[128];
		int size4;
		size4=read(clsock, tempbuff2, sizeof(tempbuff2));
		if (size4<0)
		{
			printf("error with reading\n");
			perror("write");
			exit(1);
		}
		printf("Attempt to read BEFORE fork:\n"ANSI_COLOR_YELLOW "Message: %s\nCharacters: %d\n\n" ANSI_COLOR_RESET, tempbuff2, size4);
	

		//fork off a child process
		int x = fork();
		switch (x)
		{
			case -1: //error
				perror("fork");
				exit(1);
			case 0: //child

				printf(ANSI_COLOR_RED "YOU ARE IN CHILD fork\n" ANSI_COLOR_RESET);
				
				int sizex;
				char testtestx[128];
				memset(testtestx,'\0',128);
				
				sizex=read(table[n].subscriber[(table[n].numsub)-1], testtestx, sizeof(testtestx));
				printf("Attempt to read AFTER fork, BEFORE checking to run channelserve\n");
				printf(ANSI_COLOR_YELLOW "Message: %s\nCharacters: %d"ANSI_COLOR_RESET" \n\n", testtestx, sizex);


				if(table[n].numsub==1) //fork off a child process ONLY when the client that just subscribed is the ONLY subscriber in its channel. This child process will still be running when new clients are subscribed to the channel and no new process is necessary. (this concept will need eventual updating, when subscribers can leave channels)
				{
					printf(ANSI_COLOR_MAGENTA "If you can read this, then you have just subscribed a client to the channel for the first time. That means channel serve WILL be entered. Channelserve contains the 4th and 5th print statements. So you WILL see the 4th and 5th print statements.\n" ANSI_COLOR_RESET);
					//printf("Attempting to read AFTER fork, AFTER checking to run channelserve,\nBEFORE channelserve\n");
					
					//int size;
					//char testtest[128];
					//memset(testtest,'\0',128);
					
					//size=read(table[n].subscriber[0], testtest, sizeof(testtest));
					//printf(ANSI_COLOR_YELLOW "Message: %s\nCharacters: %d"ANSI_COLOR_RESET" \n\n", testtest, size);

					//printf("About to enter channelserve on channel %d \n", n+1);
					channelserve(table[n]);
				}
				else
				{
					printf(ANSI_COLOR_RED "If you can read this, then the client that you have just subscribed to the channel is NOT the first client in that channel. That means channel serve WILL NOT be entered. Channelserve contains the 4th and 5th print statements. So you WILL NOT see the 4th and 5th print statements.\n"ANSI_COLOR_RESET);
					printf(ANSI_COLOR_RESET"\n");
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
//main ends

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//this is called by the main method when it is time to subscribe a client to it's desired channel
int subscribe_to_channel(channel c, int clsock)
{
	

	printf(ANSI_COLOR_GREEN"%s\n" ANSI_COLOR_RESET, "Entered the subscribe_to_channel method");

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
	printf("Close connection to rendezvous. Connect to channel.\n");

	//accept client's connection to channel's socket
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	clsock=accept(c.channelsocket, (struct  sockaddr *)&client_addr, &client_len);

	printf("Socket ID of client when connected to channel: %d \n", clsock);
	//printf("These two socket IDs, and the corresponding pair on the client side, \n");
	//printf("     will be the same or increment by one \n");
	//printf("     depending on where the close statement is placed in the client side code \n");
	
	printf("%s\n", "Testing connection between channel and client");

	char subscribetestbuff[128];
	memset(subscribetestbuff,'\0',128);
	int size2;
	//read of r/w #3
	size2 = read(clsock, subscribetestbuff, sizeof(subscribetestbuff));
	if (size2<0)
	{
		perror("write");
		exit(1);
	}
	//subscribetestbuff[size]='\0';
	printf(ANSI_COLOR_YELLOW"Message: %s \nCharacters: %d"ANSI_COLOR_RESET" \n",subscribetestbuff, size2);


	return clsock;

}
//subscribe ends


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
//subscribe extras end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
void leave_channel(int clsock)
{

	overwrite slot with zero
}
*/
//leave channel ends

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void channelserve(channel c)
{
	//entire channel socket is already in channel table
	printf("%s \n", "you've reached the channelserve method! hooray!");

	//this is the code for continual discussion between client and server.

	int n=c.numsub;
	printf("%s \n", "Start loop for constantly handling channel");
	while(1)
	{	
		char servetestbuff[1024];
		for (int i=0; i<n; i++) //for each subscriber
		{
			//fflush(stdout);  //not needed
			int size;
			size=read(c.subscriber[i], servetestbuff, sizeof(servetestbuff)); //read from that subscriber
			
			if(size<0) //error handling
			{
				perror("read");
				exit(1);
			}
			else if(size>0)
			{
				printf("Read from member number %d on channel with fd number %d\n", i, c.subscriber[i]);	
				printf("from client:   "ANSI_COLOR_YELLOW"%s "ANSI_COLOR_RESET"\n", servetestbuff);
				servetestbuff[size]='\0';	//null terminator
			}
			//servetestbuff[size]='\0';	//not needed here //null terminator
			//fflush(stdout);		
			//c.chanbuff="fake input";
			//strcpy(servetestbuff, "fake input");	
			//printf("%d characters were read in \n", size1);

			//printf("%s \n", "This print statement executes immediately after printing data from client");
			
			
			//this is where the server writes out to the clients the data that was read in
			//i want to get the reading in part working first before i start writing out
			
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
			
/*			//sleep(3);

			break;*/
		}
		sleep(1);
		//break;



	}

}
//channelserve ends


