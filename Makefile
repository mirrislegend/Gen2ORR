CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wunused
CFLAGS += -std=c99 -g -O2
OBJS = client.c
SOBJS=server.c

client: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o client

server:$(SOBJS)
	$(CC) $(CFLAGS) $(SOBJS) -o server

clean:
	rm -rvf *.o client
	rm -rvf *.o server