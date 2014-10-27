In Linux to compile:
gcc -o TcpServer TcpServer.c -std+=c99
gcc -o TcpClient TcpClient.c

To compile:
Use the make file and run "make all" or individually"make client" and "make server". May need to run "make clean" before

To run:
In one terminal, run ./server <number>  and in another run ./client localhost <nnumber> Enter a number such as 10000
This is able to send data multiple times. 
