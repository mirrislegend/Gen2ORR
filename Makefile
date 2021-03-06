CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wunused
CFLAGS += -std=c99 -g -O2
OBJS = tcpClient.c
SOBJS=tcpServer.c

all:
	$(CC) $(CFLAGS) $(OBJS) -o client
	$(CC) $(CFLAGS) $(SOBJS) -o server

client: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o client
	#CTRL + C to quit

server:$(SOBJS)
	$(CC) $(CFLAGS) $(SOBJS) -o server
	#CTRL + C to quit
	
clean:
	rm -rvf *.o client
	rm -rvf *.o server
