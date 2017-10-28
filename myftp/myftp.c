/**
 * AUTHOR: Clem Davies
 * DATE: 16/10/17
 * FILENAME: myftp.c
 * DESCRIPTION: The client program for myftp.
 *
 * CHANGELOG:
 * CLEM 16/10: Initialised 'hello world' main.
 * CLEM 28/10: Created client sends hard coded packets and displays the return value from server.
 *
 *
 *
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// client commands available
#define PUT  "put"
#define GET  "get"
#define PWD  "pwd"
#define LPWD "lpwd"
#define DIR  "dir"
#define LDIR "ldir"
#define CD   "cd"
#define LCD  "lcd"
#define QUIT "quit"
#define HELP "help"



void send_put()
{


}

void send_get()
{


}

void send_pwd()
{


}

void display_lpwd()
{


}

void send_dir()
{


}

void display_ldir()
{


}

void send_cd()
{


}

void display_lcd()
{


}

void send_quit()
{


}

void display_help()
{
	printf("Commands:\n");
	printf("put <filename> - send file to server\n");
	printf("get <filename> - request file from server\n");
	printf("pwd - display the current directory path on the server\n");
	printf("lpwd - display the current local directory path\n");
	printf("dir - display current directory listing on the server\n");
	printf("ldir - display current local directory listing\n");
	printf("cd - change current directory on the server\n");
	printf("lcd - change current local directory\n");
	printf("quit - terminate session\n");
	printf("help - display this information\n");
}


#define MAX_BLOCK_SIZE 64


int main(int argc, char* argv[]){

	char buf[MAX_BLOCK_SIZE], host[60];
	unsigned short port;
	struct sockaddr_in ser_addr;
	struct hostent *hp;


	/* get server host name and port number */
	if (argc==1) {
		/* assume server running on the local host and on default port */
		gethostname(host, sizeof(host));
		port = SERV_TCP_PORT;
	} else if (argc == 2) { /* use the given host name */
		strcpy(host, argv[1]);
		port = SERV_TCP_PORT;
	} else if (argc == 3) { // use given host and port for server
		strcpy(host, argv[1]);
		int n = atoi(argv[2]);
	if (n >= 1024 && n < 65536){
		port = n;
	}	else {
		printf("Error: server port number must be between 1024 and 65535\n");
		exit(1);
	}
	} else {
		printf("Usage: %s [ <server host name> [ <server listening port> ] ]\n", argv[0]);
		exit(1);
	}



	/* get host address, & build a server socket address */
	bzero((char *) &ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL){
		printf("host %s not found\n", host); exit(1);
	}
	ser_addr.sin_addr.s_addr = * (u_long *) hp->h_addr;

	/* create TCP socket & connect socket to server address */
	sd = socket(PF_INET, SOCK_STREAM, 0);
	if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0) {
		perror("client connect"); exit(1);
	}




	while (1) {
		printf("> ");

		// implement simplified command.h and token.h to parse user input into tokens of commands.

		fgets(buf, sizeof(buf), stdin);
		nr = strlen(buf);
		if (buf[nr-1] == '\n') {
			buf[nr-1] = '\0';
			nr--;
		}

		// change buf to first token / command from command.h
		if(strcmp(buf,PUT)==0){
				send_put();
		}else if(strcmp(buf,)==0){
				send_get();
		}else if(strcmp(buf,)==0){
				send_pwd();
		}else if(strcmp(buf,)==0){
				display_lpwd();
		}else if(strcmp(buf,)==0){
				send_dir();
		}else if(strcmp(buf,)==0){
				display_ldir();
		}else if(strcmp(buf,)==0){
				send_cd();
		}else if(strcmp(buf,)==0){
				display_lcd();
		}else if(strcmp(buf,)==0){
				send_quit();
		}else{
				perror("undefined command, type 'help' for help");
		}
		switch(buf){
			case PUT:
			break;
			case GET:
			break;
			case PWD:
			break;
			case LPWD:
			break;
			case DIR:
			break;
			case LDIR:
			break;
			case CD:
			break;
			case LCD:
			break;
			case QUIT:
				exit(1);
			break;
			case HELP:
				display_help();
			break;
			default:
		}

	}


}