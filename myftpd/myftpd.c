/**
 * AUTHOR: Clem Davies, Daniel Kearsley
 * DATE: 3/11/17
 * FILENAME: myftpd.c
 * DESCRIPTION: The server program for myftp.
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
#include <signal.h>

#include "stream.h"

#define FILE_BLOCK_SIZE 512

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

// error messages for OP_PUT ack codes
#define ACK_PUT_FILENAME_MSG "the server cannot accept the file as there is a filename clash"
#define ACK_PUT_CREATEFILE_MSG "the server cannot accept the file because it cannot create the named file"
#define ACK_PUT_OTHER_MSG "the server cannot accept the file due to other reasons"

// ack codes for OP_GET
#define ACK_GET_FIND '0'
#define ACK_GET_OTHER '1'

// error messages for OP_GET ack codes
#define ACK_GET_FIND_MSG "the server cannot find requested file"
#define ACK_GET_OTHER_MSG "the server cannot send the file due to other reasons"

// ack codes for OP_CD
#define ACK_CD_SUCCESS '0'
#define ACK_CD_FIND '1'


#define UNEXPECTED_ERROR_MSG "unexpected behaviour"



#define SERV_TCP_PORT   40007   /* default server listening port */
#define LOGPATH "/myftpd.log"	/* log file */


/* struct for managing socket descriptor, client id and logfile path */
typedef struct{
	int sd;	 /* socket */
	int cid; /* client id */
	char logfile[256];  /* absolute log path */
} descriptors;

/*
 * Accepts a client id, formatted output string, and va_list of args.
 * Outputs current time, client id, and passed format string to log file.
 */
void logger(descriptors *d, char* argformat, ... ){

	int fd;
	if( (fd = open(d->logfile,O_WRONLY | O_APPEND | O_CREAT,0766)) == -1 ){
		perror("unable to write to log");
		exit(0);
	}

	va_list args;
	time_t timedata;
	struct tm * timevalue;
	char timeformat[64];
	char* loggerformat;
	char* cidformat = "client %d-";
	char cidstring[64] = "";

	time(&timedata);
	timevalue = localtime ( &timedata );
	asctime_r(timevalue,timeformat); // string representation of time
	timeformat[strlen(timeformat)-1] = '-';//replace \n

	if(d->cid != 0){
		sprintf(cidstring,cidformat,d->cid);
	}

	loggerformat = (char*) malloc((strlen(timeformat) + strlen(cidstring) + strlen(argformat) + 2) * sizeof(char) );

	strcpy(loggerformat,timeformat);
	strcat(loggerformat,cidstring);
	strcat(loggerformat,argformat);
	strcat(loggerformat,"\n");

	va_start(args,argformat); // start the va_list after argformat
	vdprintf(fd,loggerformat,args);
	va_end(args); // end the va_list

	free(loggerformat);

	close(fd);
}


/*
 * Handles protocol process to send a requested file from the client to the server.
 */
void handle_put(descriptors *desc)
{
	logger(desc,"PUT");

	int filenamelength;
	char ackcode;
	char opcode;
	int fd;

	/* read filename and length */
	if( read_twobytelength(desc->sd,&filenamelength) == -1){
		logger(desc,"failed to read 2 byte length");
		return;
	}

	char filename[filenamelength + 1];

	if(read_nbytes(desc->sd,filename,filenamelength) == -1){
		logger(desc,"failed to read filename");
		return;
	}
	filename[filenamelength] = '\0';
	logger(desc,"PUT %s",filename);


	/* attempt to create file */
	ackcode = ACK_PUT_SUCCESS;
	if( (fd = open(filename,O_RDONLY)) != -1 ){
		logger(desc,"file exists: %s",filename);
		ackcode = ACK_PUT_FILENAME;
	}else	if( (fd = open(filename,O_WRONLY | O_CREAT, 0766 )) == -1 ){
		logger(desc,"cannot create file: %s",filename);
		ackcode = ACK_PUT_CREATEFILE;
	}


	/* write acknowledgement */
	if( write_code(desc->sd,OP_PUT) == -1 ){
		logger(desc,"failed to write OP_PUT");
		return;
	}

	if(write_code(desc->sd,ackcode) == -1){
		logger(desc,"failed to write ackcode:%c",ackcode);
		return;
	}

	if(ackcode != ACK_PUT_SUCCESS){
		logger(desc,"PUT completed");
		return;
	}

	/* read response from client */
	if(read_code(desc->sd,&opcode) == -1){
		logger(desc,"failed to read code");
	}
	/* expect to read OP_DATA */
	if(opcode != OP_DATA){
		logger(desc,"unexpected opcode:%c, expected: %c",opcode,OP_DATA);
		return;
	}

	int filesize;

	/* read filesize */
	if(read_fourbytelength(desc->sd,&filesize) == -1){
		logger(desc,"failed to read filesize");
		return;
	}


	int block_size = FILE_BLOCK_SIZE;
	if(FILE_BLOCK_SIZE > filesize){
		block_size = filesize;
	}
	char filebuffer[block_size];
	int nr = 0;
	int nw = 0;

	while(filesize > 0){
		if(block_size > filesize){
			block_size = filesize;
		}
		if( (nr = read_nbytes(desc->sd,filebuffer,block_size)) == -1){
			logger(desc,"failed to read bytes");
			close(fd);
			return;
		}
		if( (nw = write(fd,filebuffer,nr)) < nr ){
			logger(desc,"failed to write %d bytes, wrote %d bytes instead",nr,nw);
			close(fd);
			return;
		}
		filesize -= nw;
	}
	close(fd);
	logger(desc,"PUT success");
	logger(desc,"PUT completed");
}

/*
 * Handles protocol process to send a requested file from the server to the client.
 */
void handle_get(descriptors *desc)
{
	logger(desc,"GET");

	int fd;
	struct stat inf;
	int filesize;
	int filenamelength;
	char ackcode;


	/* read filename and length */
	if( read_twobytelength(desc->sd,&filenamelength) == -1){
		printf("failed to read 2 byte length");
		return;
	}

	char filename[filenamelength + 1];

	if(read_nbytes(desc->sd,filename,filenamelength) == -1){
		printf("failed to read filename");
		return;
	}
	filename[filenamelength] = '\0';

	logger(desc,"GET %s",filename);


	/* process the file */
	if( (fd = open(filename, O_RDONLY)) == -1){
		ackcode = ACK_GET_FIND;
		logger(desc,"%s",ACK_GET_FIND_MSG);
		if(write_code(desc->sd,OP_GET) == -1){
			logger(desc,"failed to write opcode:%c",OP_GET);
			return;
		}
		if(write_code(desc->sd,ackcode) == -1){
			logger(desc,"failed to write ackcode:%c",ackcode);
		}
		return;
	}

	if(fstat(fd, &inf) < 0) {
		logger(desc,"fstat error");
		ackcode = ACK_GET_OTHER;
		logger(desc,"%s",ACK_GET_OTHER_MSG);
		if(write_code(desc->sd,OP_GET) == -1){
			logger(desc,"failed to write opcode:%c",OP_GET);
			return;
		}
		if(write_code(desc->sd,ackcode) == -1){
			logger(desc,"failed to write ackcode:%c",ackcode);
		}
		return;
	}

	filesize = (int)inf.st_size;

	/* reset file pointer */
	lseek(fd,0,SEEK_SET);


	/* send the data */
	if( write_code(desc->sd,OP_DATA) == -1){
		logger(desc,"failed to send OP_DATA");
		return;
	}

	if(write_fourbytelength(desc->sd,filesize) == -1){
		logger(desc,"failed to send filesize");
		return;
	}

	int nr = 0;
	char buf[FILE_BLOCK_SIZE];

	while((nr = read(fd,buf,FILE_BLOCK_SIZE)) > 0){
		if ( write_nbytes(desc->sd,buf,nr) == -1){
			logger(desc,"failed to send file content");
			return;
		}
	}
	logger(desc,"GET success");
	logger(desc,"GET complete");
}

/*
 * Handles protocol process to display current directory path of server.
 */
void handle_pwd(descriptors *desc)
{
	logger(desc,"PWD");

	char cwd[1024];

	getcwd(cwd, sizeof(cwd));

	if(write_code(desc->sd,OP_PWD) == -1){
		logger(desc, "Failed to write opcode");
		return;
	}

	if(write_twobytelength(desc->sd, strlen(cwd)) == -1){
		logger(desc, "Failed to write length");
		return;
	}

	if(write_nbytes(desc->sd, cwd, strlen(cwd)) == -1){
		logger(desc, "Failed to write directory");
		return;
	}

	logger(desc, "PWD complete");
}

/*
 * Handles protocol process to display directory listing of the server.
 *
 */
void handle_dir(descriptors *desc)
{
	logger(desc,"DIR");

	char files[1024] = "";

	DIR *dir;
	struct dirent *ent;
	dir = opendir(".");
	if (dir){
		while ((ent = readdir(dir)) != NULL){
			strcat(files,ent->d_name);
			strcat(files, "\n");
		}
		files[strlen(files)-1] = '\0';
		closedir(dir);
		logger(desc, "DIR success");
	}

	if(write_code(desc->sd,OP_DIR) == -1){
		logger(desc, "Failed to write opcode");
		return;
	}

	if(write_fourbytelength(desc->sd, strlen(files)) == -1){
		logger(desc, "Failed to write length");
		return;
	}

	if(write_nbytes(desc->sd, files, strlen(files)) == -1){
		logger(desc, "Failed to write file list");
		return;
	}
	logger(desc,"DIR complete");
}

/*
 * Handles protocol process to change directory of the server.
 */
void handle_cd(descriptors *desc)
{
	logger(desc,"CD");

	int size;
	char ackcode;

	if(read_twobytelength(desc->sd,&size) == -1){
		logger(desc,"failed to read size");
		return;
	}

	char token[size+1];

	if(read_nbytes(desc->sd,token,size) == -1){
		logger(desc,"failed to read token");
		return;
	}
	token[size] = '\0';

	logger(desc,"CD %s",token);


	if(chdir(token) == 0){
		ackcode = ACK_CD_SUCCESS;
		logger(desc,"CD success");
	} else {
		ackcode = ACK_CD_FIND;
		logger(desc,"CD cannot find directory");
	}

	if(write_code(desc->sd,OP_CD) == -1){
		logger(desc, "failed to send cd");
		return;
	}

	if(write_code(desc->sd,ackcode) == -1){
		logger(desc, "failed to send ackcode");
		return;
	}
	logger(desc,"CD complete");
}

/*
 * Reads a one byte char opcode off socket connection.
 * Processes opcode recieved.
 */
void serve_a_client(descriptors *desc)
{
	char opcode;

	logger(desc,"connected");

	while (read_code(desc->sd,&opcode) > 0){

		switch(opcode){
			case OP_PUT:
				handle_put(desc);
			break;
			case OP_GET:
				handle_get(desc);
			break;
			case OP_PWD:
				handle_pwd(desc);
			break;
			case OP_DIR:
				handle_dir(desc);
			break;
			case OP_CD:
				handle_cd(desc);
			break;
			default:
				logger(desc,"invalid opcode recieved");//invalid :. disregard
			break;
		}// end switch

	}// end while

	logger(desc,"disconnected");
	return;
}

/*
 * Terminates any hanging children processes.
 */
void claim_children()
{
	pid_t pid=1;
	while (pid>0) { /* claim as many zombies as we can */
		pid = waitpid(0, (int *)0, WNOHANG);
	}
}



/*
 * Sets process as daemon.
 */
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
		setsid();		/* become session leader */
		umask(0);		/* clear file mode creation mask */

		/* catch SIGCHLD to remove zombies from system */
		act.sa_handler = claim_children; /* use reliable signal */
		sigemptyset(&act.sa_mask);       /* not to block other signals */
		act.sa_flags   = SA_NOCLDSTOP;   /* not catch stopped children */
		sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);

	}
}

/*
 * Creates a myftp server.
 * num of args) description
 * 0) current directory is used as server directory.
 * 1) user supplies directory to use as server directory.
 * log file is created and appended to in the initial server directory.
 */
int main(int argc, char* argv[])
{
	int nsd;
	pid_t pid;
	unsigned short port = SERV_TCP_PORT; /* server listening port */
	socklen_t cli_addrlen;
	struct sockaddr_in ser_addr, cli_addr;

	descriptors desc;
	desc.cid = 0;
	desc.sd = 0;

	char init_dir[256] = ".";
	char curr_dir[256] = "";

	/* get current directory */
	getcwd(curr_dir,sizeof(curr_dir));


	if( argc > 2 ) {
		printf("Usage: %s [ initial_current_directory ]\n", argv[0]);
		exit(1);
	}

	/* get the initial directory */
	if (argc == 2) {
		strcpy(init_dir,argv[1]);
	}

	if( chdir(init_dir) == -1 ){
		printf("Failed to set initial directory to: %s\n",init_dir);
		exit(1);
	}

	/* setup absolute path to logfile */
	getcwd(curr_dir,sizeof(curr_dir));
	strcpy(desc.logfile,curr_dir);
	strcat(desc.logfile,LOGPATH);

	/* make the server a daemon. */
	daemon_init();

	logger(&desc,"server initialised");

	logger(&desc,"initial dir set to %s",curr_dir);


	/* set up listening socket sd */
	if ((desc.sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server:socket"); exit(1);
	}

	/* build server Internet socket address */
	bzero((char *)&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET; // address family
	ser_addr.sin_port = htons(port); // network ordered port number
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); // any interface

	/* bind server address to socket sd */
	if (bind(desc.sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
		perror("server bind"); exit(1);
	}

	/* become a listening socket */
	listen(desc.sd, 5); // 5 maximum connections can be in queue
	logger(&desc,"myftp server now listening on port %hu",port);

	while (1) {
		/* wait to accept a client request for connection */
		cli_addrlen = sizeof(cli_addr);
		nsd = accept(desc.sd, (struct sockaddr *) &cli_addr, &cli_addrlen);
		if (nsd < 0) {
			if (errno == EINTR) continue;/* if interrupted by SIGCHLD */
			perror("server:accept"); exit(1);
		}

		/* iterate client id before forking */
		desc.cid++;

		/* create a child process to handle this client */
		if ((pid=fork()) <0) {
			perror("fork"); exit(1);
		} else if (pid > 0) {
			close(nsd);
			continue; /* parent to wait for next client */
		}else{
			/* now in child, serve the current client */
			close(desc.sd);
			desc.sd = nsd;
			serve_a_client(&desc);
			exit(0);
		}
	}
}
