/**
 * AUTHOR: Clem Davies
 * DATE: 16/10/17
 * FILENAME: myftp.c
 * DESCRIPTION: The client program for myftp.
 *
 * CHANGELOG:
 * CLEM 16/10: Initialised 'hello world' main.
 * CLEM 28/10: Created client boiler plate, sets up connection but does nothing.
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
#include <unistd.h>

#include "stream.h"


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

// opcodes
#define OP_PUT  'P'
#define OP_GET  'G'
#define OP_PWD  'A'
#define OP_DIR  'B'
#define OP_CD   'C'
#define OP_DATA 'D'

// ack codes for OP_PUT
#define ACK_PUT_SUCCESS '0'
#define ACK_PUT_FILENAME '1'
#define ACK_PUT_CREATEFILE '2'
#define ACK_PUT_OTHER '3'

// ack codes for OP_GET
#define ACK_GET_FIND '0'
#define ACK_GET_OTHER '1'

// ack codes for OP_DATA
#define ACK_DATA_ASCII '0'
#define ACK_DATA_BIN '1'

// ack codes for OP_CD
#define ACK_CD_FIND '0'
#define ACK_CD_OTHER '1'


/* temp debug function, prints response from server */
void response(int sd){
	char code;
	if( read_code(sd,&code) <= 0){
		printf("read failed\n");
		return; //connection closed
	}
	printf("Sever Output: %c\n",  code);
}

void send_put(int sd)
{
	if( write_code(sd,OP_PUT) == -1){
		printf("failed to send put\n");
	}
	response(sd);
}

void send_get(int sd)
{
	write_code(sd,OP_GET);
	response(sd);


}

void send_pwd(int sd)
{

	write_code(sd,OP_PWD);
	response(sd);

}

void display_lpwd()
{
	printf("display local pwd\n");

}

void send_dir(int sd)
{

	write_code(sd,OP_DIR);
	response(sd);

}

void display_ldir()
{
	printf("display local dir list\n");


}

void send_cd(int sd)
{
	write_code(sd,OP_CD);
	response(sd);


}

void display_lcd()
{
	printf("change local dir\n");


}

void send_quit()
{
	printf("just quit\n");

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


#define MAX_INPUT 64
#define SERV_TCP_PORT   40007   /* default server listening port */

int main(int argc, char* argv[])
{
	int sd, n, nr, nw, i=0;
	char buf[MAX_INPUT], host[60];
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
				send_put(sd);
		}else if(strcmp(buf,GET)==0){
				send_get(sd);
		}else if(strcmp(buf,PWD)==0){
				send_pwd(sd);
		}else if(strcmp(buf,LPWD)==0){
				display_lpwd();
		}else if(strcmp(buf,DIR)==0){
				send_dir(sd);
		}else if(strcmp(buf,LDIR)==0){
				display_ldir();
		}else if(strcmp(buf,CD)==0){
				send_cd(sd);
		}else if(strcmp(buf,LCD)==0){
				display_lcd();
		}else if(strcmp(buf,HELP)==0){
				display_help();
		}else if(strcmp(buf,QUIT)==0){
				send_quit();
				exit(0);
		}else{
				printf("undefined command, type 'help' for help\n");
		}

	}


}