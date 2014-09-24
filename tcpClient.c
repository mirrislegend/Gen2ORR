#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int client_serve(int new_pn, struct sockaddr_in *server_addr)
{
	
	printf("Client will now communicate along port: %d\n", new_pn);


	int x = server_addr->sin_port;
	printf("Connection with %d is terminated\n", ntohs(x));

	//changing port number
	x = new_pn;
	server_addr->sin_port = htons(x);

	//setting up new socket
	int new_sock;
	if ((new_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

//***	//connecting to new socket
	if (connect(new_sock, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
	{
		perror("connect");
		exit(1);
	}

	printf("%s \n", "Connected to new socket?");

	//char chan[]="data";
	if(write(new_sock, "If you can see this, the client successfully connected to the channel", sizeof("If you can see this, the client successfully connected to the channel"))<0)
	{
		perror("write");
		exit(1);
	}

//printf("Connected to socket on channel

	return new_sock;	
}


int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr;
	int sock;
	struct hostent *hostinfo;

	
	if (argc != 3) {
		fprintf(stderr, "Usage: client host port\n");
		exit(1);
	}

	// set up socket for TCP
	/*here we do not need to bind() because the address used does not matter
	to the server */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	// Step 2: find the internet address of the server
	/*gethostbyname() should return a pointer to the hostent structure. If it returns
	a null pointer, 0, then it means the network address was not found, error */
	if ((hostinfo = gethostbyname(argv[1])) == 0)
	{
		fprintf(stderr, "Host %s does not exist\n", argv[1]);
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr, hostinfo->h_addr_list[0],
		hostinfo->h_length);
	server_addr.sin_port = htons(atoi(argv[2]));

	// Step 3: connect to the server
	if (connect(sock, (struct sockaddr *)&server_addr,
		sizeof(server_addr)) < 0) {
		perror("connect");
		exit(1);
	}

	//get desired channel from user
	//send desired channel to server
	char buff[1024];
	printf("%s", "Input desired channel: ");
	if (fgets(buff, 1024, stdin)==NULL) //gets a newline character
	{
		perror("fgets");
		exit(1);
	}
	if( write(sock, buff, strlen(buff)-1) <0)
	{
		perror ("write");
		exit(1);
	}

	//receiving from server a new port number to communicate through
	int new_port_number;
	int x;
	if (read(sock, &x, sizeof(x))==-1)
	{
		perror("read");
		exit(1);
	}

	
	new_port_number=htons(x);

	//closing the connection to the old socket
	if (close(sock)==-1)
	{
		perror("close");
		exit(1);
	}

	printf("%s", "Close connection to rendez socket \n");

	//where the magic happens
	int new_fd;
	new_fd = client_serve(new_port_number, &server_addr);
	
	printf("%s \n", "Connected to new socket!");

	//this is where the client sends data regularly to relay
	printf("%s \n", "Writing test data to channel (aka: the new socket)");
	
	if(write(new_fd, "Test data after client_serve", sizeof("Test data after client_serve"))<0)
	{
		perror("write");
		exit(1);
	}
/*
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//time to try the real read/write stuff!

	while(1)
	{
		printf ("Input message to pass to server: ");
		char buff[1024];
		if (fgets(buff, 1024, stdin)==NULL) //gets a newline character
		{
			perror("fgets");
			exit(1);
		}
		if( write(sock, buff, strlen(buff)-1) <0)
		{
			perror ("write");
			exit(1);
		}
		
		break;
	}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
	return 0;
}
