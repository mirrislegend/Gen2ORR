//automatic reads and writes of tcpClient2 may be the reason behind current nonfunctionality. Think about this with more sleep

//error 9 before and after reading in desired channel when second client hits rendezvous
//error before subscribe (for second client) is 9
//error code 9 before open in parent
//error code 6 after open in parent

//ideas about checking file table are on hold for now


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>  
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int subscribe_to_channel(channel * c, int clsock);
void channelserve(channel *c);

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
	int i;
	for (i = 0; i < 10; i++) //each channel has
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
		int j;
		for (j=0; j<capacity; j++)
		{
			setTable[i].subscriber[j]=0;
		}
		
	}

}



//need a new experiment to check on waiting messages: client 1 sends a message and client 2 sends a message. if client 1's message gets read in and then written to everyone, what is on the connection with client 2? does reading from client 2 still yield the client 2 message? does client 2 still receive the client 1 message?
//that works!



int main(int argc, char const *argv[])
{

	
	struct stat statusbuffer;   
  	if (stat ("/tmp/myFIFO", &statusbuffer) == 0)
	{
		remove("/tmp/myFIFO");
	}

	printf("error number before mkfifo is: %d\n",errno);
//returns 0 atm
	//mkfifo("/tmp/myFIFO",  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);	
	mkfifo("/tmp/myFIFO", 0666);	

	printf("error number after mkfifo is: %d\n",errno);
//returns 0 atm
	

	//intpipefd;
	//pipefd = open("in"

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

	int option=1;
	setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

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

//~~~~~~~~~~~~~~~~this is the top of main's primary while loop

	//this loop represents the rendezvous constantly receiving incoming clients, making channels when proper, and handing 		those clients to channels 
	while(1)
	{
//when forked parent loops back to here, error number is 9. something goes wrong between the fork and here
printf("---error number at top of main loop is: %d\n",errno);

		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		//setting up client socket and accepting pending connection
		//program waits at accept. this means that the primary process of the program halts here, until a client tries to connect to the rendevous socket
		csock = accept(lsock, (struct  sockaddr *)&client_addr, &client_len);
printf("---error number after rendezvous accepts incoming socket: %d\n",errno);

		//acknowledgement
		printf("Received connection from %s. Waiting to receive channel.\n\n", inet_ntoa(client_addr.sin_addr));

		//get name from client of channel client desires
		char tempbuff1[128];
		int size3;
printf("---error number before reading in desired channel is: %d\n",errno);
		//receive channel from client
		//Read for read-write pair number 1  
		size3=read(csock, tempbuff1, sizeof(tempbuff1));
//this read should be an int between 1 and 9.  I entered 0 and it did not reject it.
printf("---error number after reading in desired channel is: %d\n",errno);	
		if (size3<0)
		{
			perror("read");
			exit(1);
		}
		
		//output to user for r/w #1
		printf("Client wants to be on channel: %d \n", atoi(tempbuff1));

		//find channel to go with desired-channel name
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

			
		printf("In main method, before fork, number of subscribers on that channel before latest client subscribes: %d \n\n", table[n].numsub);


printf("---error number before subscribe is: %d\n",errno);	
	
		//call the subscribe method
		int clsock = subscribe_to_channel(&(table[n]), csock);
		printf(ANSI_COLOR_GREEN"%s\n\n" ANSI_COLOR_RESET, "Exited the subscribe_to_channel method");

		printf("In main method, number of subscribers on that channel after latest client subscribes: %d \n\n", table[n].numsub);
printf("---error number after subscribe is: %d\n",errno);	

		printf("Close connection to rendezvous.\n");
		close(csock);
		

printf("---error number right before fork is: %d\n",errno);
		//fork off a child process
		//signal(SIGCHLD, SIG_IGN);
		int x = fork();
		int fd;
		int pipebuff;
		
		
		if (x==0) //child
		{
printf("---error number at top of child is: %d\n",errno);
			printf(ANSI_COLOR_RED "	YOU ARE IN CHILD fork\n" ANSI_COLOR_RESET);


			
/*
			//call the subscribe method
			int clsock = subscribe_to_channel(&(table[n]), csock);
			printf(ANSI_COLOR_GREEN"%s\n\n" ANSI_COLOR_RESET, "Exited the subscribe_to_channel method");

			printf("Close connection to rendezvous.\n");
			close(csock);
*/			
			if(table[n].numsub<1)
			{
				printf("numsub is less than 1. shenanigans.");
			}
			else if(table[n].numsub==1) //fork off a child process ONLY when the client that just subscribed is the ONLY subscriber in its channel. This child process will still be running when new clients are subscribed to the channel and no new process is necessary. (this concept will need eventual updating, when subscribers can leave channels)
			{
				printf(ANSI_COLOR_MAGENTA "If you can read this, then you have just subscribed a client to the channel for the first time. That means channelserve WILL be entered.\n" ANSI_COLOR_RESET);
				

/*				

		printf("error number before open in child is: %d\n",errno);	
		//returns 0 atm
				
				fd = open("/tmp/myFIFO", O_WRONLY|O_NONBLOCK);
				

		//returns 6 atm
		printf("error number after open in child is: %d\n",errno);	

				pipebuff=table[n].numsub;
				write (fd, &pipebuff, sizeof(pipebuff));
close (fd);
		printf("error number after write and close in child is: %d\n",errno);	

				usleep(10000);
				
		*/		

				channelserve(&(table[n]));
	

			}
			//else you should be in parent side of fork
			
	close(fd); //pipe is unused here
					//but pipe was existed before fork, so pipe is open in the forked off child
					//hence closing it.

	close (clsock);
	printf("---error number at bottom of child inside fork: %d\n",errno); //9 currently
		}
		else if (x>0) 		//parent
		{
			printf(ANSI_COLOR_RED "YOU ARE IN PARENT fork\n" ANSI_COLOR_RESET);
		printf("---error number at top of parent inside fork: %d\n",errno);

			if(table[n].numsub<1)
			{
				printf("numsub is less than 1. shenanigans.");
			}
			else if(table[n].numsub>1)
			{
				printf(ANSI_COLOR_RED "If you can read this, then the client that you have just subscribed to the channel is NOT the first client in that channel. That means channel serve WILL NOT be entered. %d\n"ANSI_COLOR_RESET, table[n].numsub);
				printf(ANSI_COLOR_RESET"\n");	

				
printf("---error number before open in parent is: %d\n",errno); //0 currently - not a real issue yet
				fd = open("/tmp/myFIFO", O_WRONLY|O_NONBLOCK);	
printf("---error number after open in parent is: %d\n",errno); //6 currently - not a real issue yet

				pipebuff=table[n].numsub;
				write (fd, &pipebuff, sizeof(pipebuff));
			

				usleep(10000);
				
				close(fd);	
				close (clsock);	
			}
			//else you should be in child fork

printf("---error number at bottom of parent inside fork: %d\n",errno); //9 currently - current issue
			
			
		}
		else if (x<0)
		{
		
			perror("fork");
			exit(1);
			
		
		}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~		

printf("---error number at bottom of main loop, after fork: %d\n",errno); //9 currently
	
	}
	return 0;
}
//main ends

/*
Bug hunt data:

YOU ARE IN PARENT fork
---error number at bottom of parent inside fork: 0
---error number at bottom of main loop, after fork: 9
---error number at top of main loop is: 9
YOU ARE IN CHILD fork

-try putting error checking everywhere. should theoretically yield source of errno change
 
-if bug is after fork and not in parent, must be in child. since child errno checker never displays, maybe it is happening inside channelserve?
-can't be, because all the errno checks inside channelserve are coming up clean

-maybe this error doesn't matter. consider refocusing on the second client issue
	-if subscribe method is not called in child, then it is overwriting the same variable name each loop
	-speaking of that variable, what's the point of clsock in main? does subscribe method really need a return?
*/


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//this is called by the main method when it is time to subscribe a client to it's desired channel
int subscribe_to_channel(channel * c, int clsock)
{
	

	printf(ANSI_COLOR_GREEN"%s\n" ANSI_COLOR_RESET, "Entered the subscribe_to_channel method");

	//send port number of channel to client
	printf("%s\n", "Write, to client, the socket corresponding to the client's desired channel");
	int x = htons(c->channelport);
	//write the socket to client
	//write for r/w pair #2
	if (write(clsock, &x, sizeof(x))<0)
	{
		perror("write");
		exit(1);
	}

	printf("Socket ID of client when connected to rendezvous: %d \n", clsock);

	//close(clsock);
	//printf("Close connection to rendezvous. Connect to channel.\n");

	//accept client's connection to channel's socket
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	clsock=accept(c->channelsocket, (struct  sockaddr *)&client_addr, &client_len);

	printf("Socket ID of client when connected to channel: %d \n", clsock);
	//printf("These two socket IDs, and the corresponding pair on the client side, \n");
	//printf("     will be the same or increment by one \n");
	//printf("     depending on where the close statement is placed in the client side code \n");
	
	printf("%s\n", "Testing connection between channel and client");


	int m;
	for (m=0; m<10; m++)
	{
		
		
		if (c->subscriber[m]==0)
		{
			c->subscriber[m]=clsock;

			printf("Channel %d, subscriber #%d has fd=%d\n", c->channel_name, m+1, c->subscriber[m]);

			break;
		}
		else
		{
			printf("Channel %d, subscriber #%d has fd=%d\n", c->channel_name, m+1, c->subscriber[m]);
		}
		

	}
	
	c->numsub=(c->numsub)+1;  //just subscribed a member, so increment the number of subscribers

	printf("Number of subscribers after subscription of latest client: %d \n\n", c->numsub);


	//this is code for test messages - debugging only
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
	printf(ANSI_COLOR_YELLOW"%s\n"ANSI_COLOR_RESET"\n", subscribetestbuff);




	return clsock;

}
//subscribe ends

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
void leave_channel(int clsock)
{

	overwrite slot with zero
}
*/
//leave channel ends

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void channelserve(channel (*c))
{
printf("---error number at start of channelserve is: %d\n",errno); 
//returns 0 atm
	int fd=-2;
	int pipebuff=1;
	int numsubscribersparent=1;



	//entire channel socket is already in channel table
	printf("-%s \n", "You've reached the channelserve method!");

	

	//this is the code for continual discussion between client and server.

	printf("-%s \n\n", "Start loop for constantly handling channel");
	int n;
	while(1)
	{	
		printf("-This is the top of the channelserve loop\n");

		usleep(10000);
		fd = open("/tmp/myFIFO", O_RDONLY|O_NONBLOCK);
		read(fd, &pipebuff, sizeof(pipebuff));
		//close(fd);
printf("---error number after read in channelserve is: %d\n",errno);
//returns 0 atm

		
		numsubscribersparent=pipebuff;
usleep(10000);


		(*c).numsub=numsubscribersparent;

		
		printf("-Number of subscribers according to child: %d \n", (*c).numsub);

		
		char servetestbuff[128];
		
		memset( servetestbuff, '\0', 128);
		//servetestbuff[0]='z';
		printf("-Cleared channel read-in buffer (which will later be removed, with only one buffer for input and output).\n");
		
		
		memset((*c).chanbuff, '\0', 128);
		//(*c).chanbuff[0]='z';
		printf("-Cleared channel buffer.\n");

		n=(*c).numsub;	
		printf("-There are %d clients currently subscribed to this channel.\n", n);
		

		int i;
		for (i=0; i<n; i++) //for each subscriber
		{
			printf("-Reading from subscriber #%d. \n", i);
			int size;
			size=read((*c).subscriber[i], servetestbuff, sizeof(servetestbuff)); //read from that subscriber
			
			if(size<0) //error handling
			{
				perror("read");
				exit(1);
			}
			//else if (size==0)
			//{
			//	sprintf(c.chanbuff, "");
			//}
			else if(size>0)
			{
				printf("-Read(past tense) %d characters from member #%d on channel with fd number %d\n", size, i, (*c).subscriber[i]);	
				printf("-from client: "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"\n", servetestbuff);

								
				//ends string properly
				//makes the string "empty" if nothing is read in aka if size==0
				//servetestbuff[size]='z';

				//fill chanbuff for broadcast
				sprintf((*c).chanbuff, "Received from subscriber# %d: "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"\n",i, servetestbuff);
				printf("-Server side is broadcasting: "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"", (*c).chanbuff);

				//if(size!=0) //if something is read in
				//{
					int j;
					for (j=0; j<n; j++) //for each member
					{
						//if (j!=i) //that is not the current member  

						//{
							printf("-Write to member #%d: \n", j);
							if (write((*c).subscriber[j], (*c).chanbuff, strlen((*c).chanbuff))<0) //write to that member
							{
								perror ("read");
								exit(1);
							}
							else{
								printf("-Written: "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET" \n", (*c).chanbuff);
							}

						//}
					}
				//}
		
			}

		
			//printf("%s \n", "This print statement executes immediately after printing data from client");
			
			
			//this is where the server writes out to the clients the data that was read in
			//i want to get the reading in part working first before i start writing out


		/*	if((*c).chanbuff[0]!='z') //if something is read in
			{
				int j;
				for (j=0; j<n; j++) //for each member
				{
					//if (j!=i) //that is not the current member  

					//{
						printf("Write to member #%d: \n", j);
						if (write((*c).subscriber[j], (*c).chanbuff, strlen((*c).chanbuff))<0) //write to that member
						{
							perror ("read");
							exit(1);
						}
						else{
							printf("Written: "ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET" \n", (*c).chanbuff);
						}

					//}
				}
			}*/
			
/*			//sleep(3);

			break;*/
		}
		sleep(1);

		//break;

	}  //ends while(1)

}
//channelserve ends

