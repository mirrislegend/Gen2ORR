#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[ ]) {
	struct sockaddr_in my_addr;
	int lsock;
	void serve(int);
	if (argc != 2) {
		fprintf(stderr, "Usage: server port\n");
		exit(1);
	}

	// Step 1: establish a socket for TCP
	if ((lsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	/* Step 2: set up our address */
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(atoi(argv[1]));

	/* Step 3: bind the address to our socket */
	if (bind(lsock, (struct sockaddr *)&my_addr,
		sizeof(my_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	/* Step 4: put socket into �listening mode� */
	//only necessary for connection-oriented data models
	if (listen(lsock, 5) < 0) {
		/*lsock is the (int) socket descriptor, 5 is the (int) backlog
		 the number of pending connections that can be queued at any time*/
		perror("listen");
		/*An error occurs if -1 is returned*/
		exit(1);
	}

	/*While loop runs while 1 is true i.e. forever*/
	while (1) {
		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		/* Step 5: receive a connection */
		/*accept() creates a new socket (csock) with the same properties as the
		original (lsock) and returns a file descriptor for it (0 for standard input,
		1 for standard output, and 2 for standard error).
		Once a connection is made, the pending connection is removed from the queue.
		- lsock is the listening socket that has the connection queued
		- * &client_addr is a pointer to a sockaddr structure to receive the client's
		address information
		- &client_len is a pointer to a socklen_t location that specifies the size
		of the client address structure passed to accept()
		NOTE: dataggram sockets do not require processing by aaccept() since the
		receiver can immediately respond to the request using the listening socket*/
		csock = accept(lsock,
			(struct sockaddr *)&client_addr, &client_len);
		printf("Received connection from %s\n",		
			inet_ntoa(client_addr.sin_addr));
		/*Below is a forked off process to handle the connection on the new socket.
		- If the file descriptor returned from accept is -1, we have an error.
		- If the file descriptor returned is 0, that's good! We call serve() to handle
		the connection using the file descriptor as the method input*/
			switch (fork( )) {
			case -1:
				perror("fork");
				exit(1);
			case 0:
				// Step 6: create a new process to handle connection
				serve(csock);
				break;
			default:	
				close(csock);
				break;
		}
	}
	//line below is merely to terminate program
	return 0;
}

void serve(int fd) {
	char buf[1024];
	int count;

	// Step 7: read incoming data from connection
	/* read() returns 0 on end-of-file (which is good) and -1 in the event of an error
	write() uses 1 as the handle because 1 is the file descriptor for standard output
	and is thus relevant for write()*/
	while ((count = read(fd, buf, 1024)) > 0) {
		write(1, buf, count);
	}
	if (count == -1) {
		perror("read");
		exit(1);
	}
	printf("connection terminated\n");
}
