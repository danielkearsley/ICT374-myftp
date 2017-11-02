/**
 * AUTHOR: Clem Davies, Daniel Kearsley
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

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "stream.h"
#include "token.h"

#define FILE_BLOCK_SIZE 512
#define MAX_CMD_INPUT 64
#define SERV_TCP_PORT   40007   /* default server listening port */

// client commands available
#define CMD_PUT  "put"
#define CMD_GET  "get"
#define CMD_PWD  "pwd"
#define CMD_LPWD "lpwd"
#define CMD_DIR  "dir"
#define CMD_LDIR "ldir"
#define CMD_CD   "cd"
#define CMD_LCD  "lcd"
#define CMD_QUIT "quit"
#define CMD_HELP "help"

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

// error messages for OP_PUT ack codes
#define ACK_PUT_FILENAME_MSG "the server cannot accept the file as there is a filename clash"
#define ACK_PUT_CREATEFILE_MSG "the server cannot accept the file because it cannot create the named file"
#define ACK_PUT_OTHER_MSG "the server cannot accept the file due to other reasons"

// ack codes for OP_GET
#define ACK_GET_FIND '0'
#define ACK_GET_OTHER '1'

// error messages for OP_GET ack codes
#define ACK_GET_OTHER_MSG "the server cannot send the file due to other reasons"

// ack codes for OP_DATA
#define ACK_DATA_ASCII '0'
#define ACK_DATA_BIN '1'

// ack codes for OP_CD
#define ACK_CD_SUCCESS '0'
#define ACK_CD_FIND '1'

// error messages for OP_CD ack codes
#define ACK_CD_OTHER_MSG "the server cannot change directory due to other reasons"


#define UNEXPECTED_ERROR_MSG "unexpected behaviour"




/* temp debug function, prints response from server */
void response(int sd){
	char code;
	if( read_code(sd,&code) <= 0){
		printf("read failed\n");
		return; //connection closed
	}
	printf("Sever Output: %c\n",  code);
}


char find_filetype(int fd)
{
	char filetype = ACK_DATA_ASCII;

	char buf[FILE_BLOCK_SIZE];
	int nr = 0;
	int totalr = 0;
	int found_null = 0;
	while((!found_null) && (nr = read(fd,buf,FILE_BLOCK_SIZE - totalr)) > 0){
		printf("inside filtype loop. nr: %d\n",nr);
		for(int i = 0; (i < nr) && (!found_null); i++){
			found_null = buf[i] == '\0';
		}
		totalr += nr;
	}

	if(found_null)
		filetype = ACK_DATA_BIN;

	return filetype;
}

void send_put(int sd, char *filename)
{
	int fd;
	struct stat inf;
	int filesize;
	int blocksize;
	int filenamelength = strlen(filename);
	char filetype;

	char opcode;
	char ackcode;


	// char *content = "x file static content\nend";//debug

	/*
		process the file before initiating put protocol
	*/
	if( (fd = open(filename, O_RDONLY)) == -1){
		printf("failed to open file: %s\n",filename);
		return;
	}

	printf("%s: ", filename);
	if(fstat(fd, &inf) < 0) {
		printf("fstat error\n");
		return;
	}

	filesize = (int)inf.st_size;
	blocksize = (int)inf.st_blksize;//?
	filetype = find_filetype(fd);

	printf("nbytes: %d\n",filesize); //debug
	printf("blocksize: %d\n",blocksize); //debug
	printf("filetype: %c\n",filetype);


	lseek(fd,0,SEEK_SET);

	/*
		initiate put protocol
	*/
	if( write_code(sd,OP_PUT) == -1){
		printf("failed to send put");
		return;
	}

	if( write_twobytelength(sd,filenamelength) == -1){
		printf("failed to send length\n");
		return;
	}
	printf("sent length %d\n",filenamelength);//debug

	if( write_nbytes(sd,filename,filenamelength) <= 0 ){
		printf("failed to send filename\n");
		return;
	}
	printf("sent filename %s\n",filename);//debug


	if(read_code(sd,&opcode) == -1){
		printf("failed to read opcode\n");
	}
	if(opcode != OP_PUT){
		printf("unexpected opcode\n");
	}
	printf("opcode processed\n"); //debug


	if(read_code(sd,&ackcode) == -1){
		printf("failed to read ackcode\n");
	}

	switch(ackcode){
		case ACK_PUT_SUCCESS://continue
		break;
		case ACK_PUT_FILENAME:
			printf("%s\n",ACK_PUT_FILENAME_MSG);
			return;
		break;
		case ACK_PUT_CREATEFILE:
			printf("%s\n",ACK_PUT_CREATEFILE_MSG);
			return;
		break;
		case ACK_PUT_OTHER:
			printf("%s\n",ACK_PUT_OTHER_MSG);
			return;
		break;
		default:
			printf("%s\n",UNEXPECTED_ERROR_MSG);
			return;
		break;
	}

	printf("ackcode processed\n");//debug


	if( write_code(sd,OP_DATA) == -1){
		printf("failed to send OP_DATA\n");
	}
	printf("sent OP_DATA\n");//debug


	if(write_code(sd,filetype) == -1){
		printf("failed to send ACK_DATA_ASCII\n");
	}
	printf("sent filetype\n");//debug


	if(write_fourbytelength(sd,filesize) == -1){
		printf("failed to send filesize\n");
	}
	printf("sent filesize:%d\n",filesize);//debug


	int nr = 0;
	char buf[FILE_BLOCK_SIZE];
	printf("nead to read and send %d bytes\n",filesize);

	while((nr = read(fd,buf,FILE_BLOCK_SIZE)) > 0){
			printf("read %d bytes\n",nr);
		if ( write_nbytes(sd,buf,nr) == -1){
			printf("failed to send file content\n");
			return;
		}
		printf("sent %d bytes\n",nr);
	}
	printf("sent file content nr: %d\n",nr);//debug

	// if(write_nbytes(sd,content,filesize) == -1){
	// 	printf("failed to send file content\n");
	// }

	response(sd); // debug



	// if( write_twobytelength(sd,filenamelength) == -1 ){
	// 	printf("failed to send filesize\n");
	// }
	// if( write_filename(sd,filename,filenamelength) == -1){

	// }

	// read_code(sd,);

	// write_code(sd,OP_DATA);

	// write_code(sd,ACK_DATA_ASCII);

	// write_fourbytelength(sd,filesize);

	// write_file(sd,filedescriptor);


	// response(sd);
}

void send_get(int sd, char *token)
{
	printf("%s\n", token);
	write_code(sd,OP_GET);
	response(sd);
}

void send_pwd(int sd, char *token)
{
	char opcode;
	int filesize;

	if(write_code(sd,OP_PWD) == -1){
		printf("Failed to send pwd\n");
		return;
	}

	if(read_code(sd,&opcode) == -1){
		printf("Failed to read opcode\n");
		return;
	}

	if(opcode != OP_PWD){
		printf("Invalid opcode:pwd %c\n",opcode);
		return;
	}

	if(read_twobytelength(sd, &filesize) == -1){
		printf("Failed to read filesize\n");
		return;
	}

	char directory[filesize+1];

	if(read_nbytes(sd, directory, filesize) == -1){
		printf("Failed to read directory\n");
		return;
	}

	directory[filesize] = '\0';
	printf("%s\n", directory);
	//response(sd);

}

//Displays the current local directory
void display_lpwd()
{
	char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
	//printf("display local pwd\n");

}

void send_dir(int sd, char *token)
{
	char opcode;
	int filesize;

	if(write_code(sd,OP_DIR) == -1){
		printf("Failed to send dir\n");
		return;
	}

	if(read_code(sd,&opcode) == -1){
		printf("Failed to read opcode\n");
		return;
	}

	if(opcode != OP_DIR){
		printf("Invalid opcode: dir %c\n",opcode);
		return;
	}

	if(read_fourbytelength(sd, &filesize) == -1){
		printf("Failed to read filesize\n");
		return;
	}

	char directory[filesize+1];

	if(read_nbytes(sd, directory, filesize) == -1){
		printf("Failed to read directory list\n");
		return;
	}
	directory[filesize] = '\0';

	printf("%s\n", directory);

}

//Chenges the current local directory
void display_ldir(char *token)
{
	if(token == NULL){
		token = ".";
	}
	DIR *d;
	struct dirent *dir;

	d = opendir(token);
	if (d){
    	while ((dir = readdir(d)) != NULL){
	      printf("%s\n", dir->d_name);
	    }

	    closedir(d);
	}
}

void send_cd(int sd, char *token)
{
	char opcode;
	char ackcode;
	int filesize;
	int length = strlen(token);

	if(write_code(sd,OP_CD) == -1){
		printf("Failed to send cd\n");
		return;
	}

	if(write_twobytelength(sd, length) == -1){
		printf("Failed to write length\n");
		return;
	}

	if(write_nbytes(sd, token, sizeof(token)) == -1){
		printf("Failed to write directory name\n");
	}

	if(read_code(sd,&opcode) == -1){
		printf("Failed to read opcode\n");
		return;
	}

	if(opcode != OP_CD){
		printf("Invalid opcode:cd %c\n",opcode);
		return;
	}

	if(read_code(sd,&ackcode) == -1){
		printf("Failed to read ackcode\n");
		return;
	}

	if(ackcode == ACK_CD_SUCCESS){
		return;
	}

	if(ackcode == ACK_CD_FIND){
		printf("the server cannot find the directory\n");
		return;
	}


}

void display_lcd(char *token)
{
	chdir(token);
}

void display_quit()
{
	printf("Session terminated\n");
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

int main(int argc, char* argv[])
{
	int sd, nr;
	char buf[MAX_CMD_INPUT], host[60];
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

	} else if (argc == 3) { /* use given host and port for server */
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


 	char *tokens[2];

	while (1) {
		printf("> ");

		/* read user input and tokenise */
		fgets(buf, sizeof(buf), stdin);
		nr = strlen(buf);
		if (buf[nr-1] == '\n') {
			buf[nr-1] = '\0';
			nr--;
		}
		tokenise(buf, tokens);

		printf("input: %s\n",buf);

		if(strcmp(tokens[0],CMD_PUT)==0){
			send_put(sd, tokens[1]);

		}else if(strcmp(tokens[0],CMD_GET)==0){
			send_get(sd, tokens[1]);

		}else if(strcmp(tokens[0],CMD_PWD)==0){
			send_pwd(sd, tokens[0]);

		}else if(strcmp(tokens[0],CMD_LPWD)==0){
			display_lpwd();

		}else if(strcmp(tokens[0],CMD_DIR)==0){
			send_dir(sd, tokens[0]);

		}else if(strcmp(tokens[0],CMD_LDIR)==0){
			display_ldir(tokens[1]);

		}else if(strcmp(tokens[0],CMD_CD)==0){
			send_cd(sd, tokens[1]);

		}else if(strcmp(tokens[0],CMD_LCD)==0){
			display_lcd(tokens[1]);

		}else if(strcmp(tokens[0],CMD_HELP)==0){
			display_help();

		}else if(strcmp(tokens[0],CMD_QUIT)==0){
			display_quit();
			exit(0);

		}else{
				printf("undefined command, type 'help' for help\n");
		}

	}


}