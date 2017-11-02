/**
 * AUTHOR: Clem Davies, Daniel Kearsley
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
#include <dirent.h> 
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



#define SERV_TCP_PORT   40007   /* default server listening port */
#define LOGPATH "./myftpd.log"	/* log file */


/*
 * Accepts a client id, formatted output string, and va_list of args.
 * Outputs current time, client id, and passed format string to log file.
 */
void logger(int cid, char* argformat, ... ){
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
  char* cidformat = "client %d-";
  char cidstring[64];

  time(&timedata );
  timevalue = localtime ( &timedata );
	asctime_r(timevalue,timeformat); // string representation of time
	timeformat[strlen(timeformat)-1] = '-';//replace \n

	if(cid != 0){
 		sprintf(cidstring,cidformat,cid);
	}

  loggerformat = (char*) malloc((strlen(timeformat) + strlen(cidstring) + strlen(argformat) + 2) * sizeof(char) );

  strcpy(loggerformat,timeformat);
  strcat(loggerformat,cidstring);
  strcat(loggerformat,argformat);
  strcat(loggerformat,"\n");

  va_start(args,argformat); // start the va_list after argformat
	vfprintf(logfile,loggerformat,args);
	va_end(args); // end the va_list

	free(loggerformat);

	fclose(logfile);
}


/*
 *
 *
 */
void handle_put(int sd, int cid)
{
	logger(cid,"PUT");

	int *filenamelength;
	char ackcode;
	char opcode;

	// finish reading put message

	if( read_twobytelength(sd,&filenamelength) == -1){
		logger(cid,"failed to read 2 byte length");
	}

	int size = (*filenamelength);
	size+=1;
	logger(cid,"filename size: %d",size);

	char filename[size];
	logger(cid,"debug");

	logger(cid,"filename length: %d",*filenamelength);
	if(read_nbytes(sd,filename,*filenamelength) == -1){
		logger(cid,"failed to read filename");
	}
	filename[*filenamelength] = '\0';
	logger(cid,"filename: %s",filename);


	// write acknowledgement

	if( write_code(sd,OP_PUT) == -1){
		logger(cid,"failed to write OP_PUT");
	}
	logger(cid,"returned OP_PUT");

	/*
		if( filename exists in curr dir ){
			ackcode = ACK_PUT_FILENAME
		} else if( filename can't be created in cur dir ){
			ackcode  = ACK_PUT_CREATEFILE
		}else if (other fail?){
			ackcode = ACK_PUT_OTHER
		}else{
			ackcode = ACK_PUT_SUCCESS
		}
	*/
	ackcode = ACK_PUT_SUCCESS; // debug statement

	if(write_code(sd,ackcode) == -1){
		logger(cid,"failed to write ackcode:%c",ackcode);
	}
	logger(cid,"returned ackcode:%c",ackcode);


	// expect to read data message

	if(read_code(sd,&opcode) == -1){
		logger(cid,"failed to read code");
	}
	if(opcode != OP_DATA){
		logger(cid,"unexpected opcode:%c, expected: %c",opcode,OP_DATA);
	}

	char filetype;
	if(read_code(sd,&filetype) == -1){
		logger(cid,"failed to read filetype");
	}
	logger(cid,"filetype: %c",filetype);

	int *filesize;

	if(read_fourbytelength(sd,&filesize) == -1){
		logger(cid,"failed to read filesize");
	}
	logger(cid,"filesize:%d",*filesize);

	char content[(*filesize)+1];

	if(read_nbytes(sd,content,*filesize) == -1){
		logger(cid,"failed to read file content");
	}
	// content[*filesize] = '\0';
	logger(cid,"content:%s",content);//debug

	//debug - write ack code
	if(write_code(sd,ACK_PUT_SUCCESS) == -1){
		logger(cid,"failed to write ackcode:%c",ACK_PUT_SUCCESS);
	}
	logger(cid,"returned ackcode:%c",ACK_PUT_SUCCESS);

}

/*
 *
 *
 */
void handle_get(int sd, int cid)
{
	logger(cid,"GET");
	write_code(sd,ACK_PUT_SUCCESS);

}

/*
 *
 *
 */
void handle_pwd(int sd, int cid)
{
	logger(cid,"PWD");
	write_code(sd,ACK_PUT_SUCCESS);

}

/*
 *
 *
 */
void handle_dir(int sd, int cid)
{
	logger(cid,"DIR");
	write_code(sd,ACK_PUT_SUCCESS);

}

/*
 *
 *
 */
void handle_cd(int sd, int cid)
{
	logger(cid,"CD");
	write_code(sd,ACK_PUT_SUCCESS);

}

/*
 *
 *
 */
void handle_data(int sd, int cid)
{
	logger(cid,"DATA");
	write_code(sd,ACK_PUT_SUCCESS);

}




/*
 *
 *
 */
void serve_a_client(int sd,int cid)
{
	char opcode;

	logger(cid,"connected");

	while (read_code(sd,&opcode) > 0){

		switch(opcode){
			case OP_PUT:
				handle_put(sd,cid);
			break;
			case OP_GET:
				handle_get(sd,cid);
			break;
			case OP_PWD:
				handle_pwd(sd,cid);
			break;
			case OP_DIR:
				handle_dir(sd,cid);
			break;
			case OP_CD:
				handle_cd(sd,cid);
			break;
			case OP_DATA:
				handle_data(sd,cid);// not needed here?
			break;
			default:
				logger(cid,"invalid opcode");//invalid :. disregard
			break;
		}// end switch

	}// end while

	logger(cid,"disconnected");
	return;
}

//Show current directory
void ShowCurDir(){
	char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

//Show files in elected directory
void ShowFiles(char *token){
	if(token == NULL){
		token = ".";
	}
	FILE *d;
  	struct dirent *dir;
  	d = opendir(token);
	if (d){
    	while ((dir = readdir(d)) != NULL){
	      printf("%s\n", dir->d_name);
	    }

	    closedir(d);
	}
}

//Change current directory
void ChangeDir(char *token){
	chdir(token);
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
		/* child */
		logger(0,"server initialised");
		setsid();		/* become session leader */


		// move chdir outside daemon_init()
		// set by user from argv in main, or current if not existent
		char current_dir[128];
		getcwd(current_dir,sizeof(current_dir));
		chdir(current_dir);	/* change working directory */
		logger(0,"dir set to %s",current_dir);



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
	int cid = 0; // client session id


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


	/* set up listening socket sd */
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server:socket"); exit(1);
	}

	/* build server Internet socket address */
	bzero((char *)&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET; // address family
	ser_addr.sin_port = htons(port); // network ordered port number
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); // any interface

	/* bind server address to socket sd */
	if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
		perror("server bind"); exit(1);
	}

	/* become a listening socket */
	listen(sd, 5); // 5 maximum connections can be in queue
	logger(0,"myftp server now listening on port %hu",port);

	while (1) {
		/* wait to accept a client request for connection */
		cli_addrlen = sizeof(cli_addr);
		nsd = accept(sd, (struct sockaddr *) &cli_addr, &cli_addrlen);
		if (nsd < 0) {
			if (errno == EINTR) continue;/* if interrupted by SIGCHLD */
			perror("server:accept"); exit(1);
		}
		// iterate client id.
		cid++;

		/* create a child process to handle this client */
		if ((pid=fork()) <0) {
			perror("fork"); exit(1);
		} else if (pid > 0) {
			close(nsd);
			continue; /* parent to wait for next client */
		}else{
			/* now in child, serve the current client */
			close(sd);
			serve_a_client(nsd,cid);
			exit(0);
		}
	}



}