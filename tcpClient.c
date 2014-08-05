#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int client_serve(int new_pn, struct sockaddr_in *server_addr, int sock)
{
	printf("Client will now comminucate along port: %d\n", new_pn);

	//closing the connection to the old socket
	if (close(sock)==-1)
	{
		perror("close");
		exit(1);
	}
	printf("Connection with %d is terminated\n", ntohs(server_addr->sin_port));

	//changing port number
	server_addr->sin_port = htons(new_pn);

	//setting up new socket
	if ((new_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	//connecting to new socket
	if (connect(new_sock, (struct sockaddr *)&server_addr,
		sizeof(server_addr)) < 0) {
		perror("connect");
		exit(1);
	}
	/*
	At this point, you will want to have code that confirms the client can now communicate along the new socket. Think of reading and writing.
	Also, we return the new socket because its use will probably come in handy later.
	*/
	return new_sock;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr;
	int sock;
	struct hostent *hostinfo;

	char buf[1024];
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
	if ((hostinfo = gethostbyname(argv[1])) == 0) {
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

	//giving the client a new port number to comminucate through
	int new_port_number;
	if (read(sock, &new_port_number, sizeof(new_port_number))==-1)
	{
		perror("read");
		exit(1);
	}

	//where the magic happens
	int new_fd = client_serve(new_port_number, &server_addr, sock);

	return(0);
}
