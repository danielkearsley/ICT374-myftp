/**
 * AUTHOR: Clem Davies
 * DATE: 16/10/17
 * FILENAME: myftpd.c
 * DESCRIPTION: The server program for myftp.
 *
 * CHANGELOG:
 * CLEM 16/10: Initialised 'hello world' main.
 * CLEM 28/10: Created server daemon that returns what it is sent from a client.
 *
 *
 *
 *
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <time.h>
#include <stdarg.h>

#include "stream.h"


// packet opcode constants
#define put  'P'
#define get  'G'
#define pwd  'A'
#define dir  'B'
#define cd   'C'
#define data 'D'



#define SERV_TCP_PORT   40007   /* default server listening port */
#define LOGPATH "./myftpd.log"	/* log file */

void logger(char* argformat, ... ){
	FILE* logfile;
  if( (logfile = fopen(LOGPATH,"a")) == NULL ){
    perror("unable to write to log");
    exit(0);
  }

	va_list args;
  time_t timedata;
  struct tm * timevalue;
  char timeformat[64];
  char* loggerformat;

  time(&timedata );
  timevalue = localtime ( &timedata );
	asctime_r(timevalue,timeformat); // string representation of time
	timeformat[strlen(timeformat)-1] = '-';//remove \n

  loggerformat = (char*) malloc((strlen(timeformat) + strlen(argformat) + 2) * sizeof(char) );

  strcpy(loggerformat,timeformat);
  strcat(loggerformat,argformat);
  strcat(loggerformat,"\n");

  va_start(args,argformat); // start the va_list after argformat
	vfprintf(logfile,loggerformat,args);
	va_end(args); // end the va_list

	free(loggerformat);

	fclose(logfile);
}


/*
	Clem 28/10/17
		Returns the data sent to server.
*/
void serve_a_client(int sd)
{
	// int nr, nw;
	// char buf[MAX_BLOCK_SIZE];

	char opcode;

	while (1){
		if( (opcode = read_opcode(sd,&opcode)) <= 0){
			return; //connection closed
		}

		switch(opcode){
			case put:
				printf("opcode put\n");
			break;
			case get:
				printf("opcode get\n");
			break;
			case pwd:
				printf("opcode pwd\n");
			break;
			case dir:
				printf("opcode dir\n");
			break;
			case cd:
				printf("opcode cd\n");
			break;
			case data:
				printf("opcode data\n");
			break;
			default:
				//invalid :. disregard
			break;
		}

		// /* read data from client */
		// if ((nr = readn(sd, buf, sizeof(buf))) <= 0)
		// 	return;    connection broken down

		// /* process data */
		// buf[nr] = '\0';

		// /* send results to client */
		// nw = writen(sd, buf, nr);
	}
}



void claim_children()
{
	pid_t pid=1;
	while (pid>0) { /* claim as many zombies as we can */
		pid = waitpid(0, (int *)0, WNOHANG);
	}
}


void daemon_init(void)
{
	pid_t pid;
	struct sigaction act;

	if ( (pid = fork()) < 0) {
		perror("fork"); exit(1);
	} else if (pid > 0) {
		/* parent */
		printf("myftpd PID: %d\n", pid);
		exit(0);
	}else{
		logger("server initialised");
		/* child */
		setsid();		/* become session leader */
		char current_dir[128];
		getcwd(current_dir,sizeof(current_dir));
		chdir(current_dir);	/* change working directory */
		logger("dir set to %s",current_dir);

		umask(0);		/* clear file mode creation mask */

		/* catch SIGCHLD to remove zombies from system */
		act.sa_handler = claim_children; /* use reliable signal */
		sigemptyset(&act.sa_mask);       /* not to block other signals */
		act.sa_flags   = SA_NOCLDSTOP;   /* not catch stopped children */
		sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);
	}
}


int main(int argc, char* argv[])
{
	int sd, nsd;
	pid_t pid;
	unsigned short port;   // server listening port
	socklen_t cli_addrlen;
	struct sockaddr_in ser_addr, cli_addr;


	/* get the port number */
	if (argc == 1) {
		port = SERV_TCP_PORT;
	} else if (argc == 2) {
		int n = atoi(argv[1]);
		if (n >= 1024 && n < 65536){
			port = n;
		}else {
			printf("Error: port number must be between 1024 and 65535\n");
			exit(1);
		}
	} else {
		printf("Usage: %s [ server listening port ]\n", argv[0]);
		exit(1);
	}

	// make the server a daemon.
	daemon_init();

	logger("listening on port %hu",port);

	/* set up listening socket sd */
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server:socket"); exit(1);
	}

	/* build server Internet socket address */
	bzero((char *)&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(port);
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* bind server address to socket sd */
	if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
		perror("server bind"); exit(1);
	}

	/* become a listening socket */
	listen(sd, 5);


	while (1) {
		/* wait to accept a client request for connection */
		cli_addrlen = sizeof(cli_addr);
		nsd = accept(sd, (struct sockaddr *) &cli_addr, &cli_addrlen);
		if (nsd < 0) {
			if (errno == EINTR) continue;/* if interrupted by SIGCHLD */
			perror("server:accept"); exit(1);
		}

		/* create a child process to handle this client */
		if ((pid=fork()) <0) {
			perror("fork"); exit(1);
		} else if (pid > 0) {
			close(nsd);
			continue; /* parent to wait for next client */
		}else{
			/* now in child, serve the current client */
			close(sd);
			serve_a_client(nsd);
			exit(0);
		}
	}



}