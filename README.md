# ICT374-myftp
Murdoch University group project for ICT374. Implement an FTP client and server in C on linux.

Designed and implemented by:
Clem and Daniel


## MYFTP protocol spec

### Syntax - formats of packets

	RRQ, WRQ
		| 2B     | nB       | 0 | 0-1  | 0 |
		| opcode | filename | 0 | mode | 0 |
	CMD
		| 2B     | nB      | 0 |
		| opcode | command | 0 |
	DATA
		| 2B     | 0-512    |
		| opcode | payload  |
	ACK
		| 2B     |
		| opcode |
	ERROR
		| 2B     | 1B         | 0 |
		| opcode | error code | 0 |


### Semantics - packet types, file mode supported, commands supported, error codes

	RRQ
		- Client sends either a RRQ or WRQ with a filename and mode flag.
		- Server processes request and returns a DATA packet or ERR packet.
		- Server continues to send DATA packets until end of file.
	WRQ
		- Client sends a WRQ packet with filename and mode.
		- Server returns either an ACK packet or ERR packet back.
		- If ACK is received on client, client can begin to send DATA packets to server until no more DATA packets to send.
		Modes:
			0) ascii
			1) binary

	CMD
		- Client sends a CMD packet with one of the following commands.
		- Server processes request and if successful returns a DATA packet, ACK packet, or ERR packet on error.

		Commands: cmd name, desc, response packet
			-PWD, Display current directory of client, DATA packet with current client directory string.
			-LPWD, Display current directory of server, DATA packet with current server directory string.
			-DIR, Display current directory listing of client, DATA packet with current client directory listing.
			-LDIR, Display current directory listing of server, DATA packet with current server directory listing.
			-CD [dirname], Changes current directory of client, ACK
			-LCD [dirname], Changes current directory of server,ACK
			-QUIT, terminates myftp session, ACK

	DATA
		- Used by client and server to send chunked data of files to the other.
		- If payload size is 512B then there is more data to send.
		- If payload size is 0-511B then there is no more data to send.

	ACK
		- A 2B packet used to acknowledge request has been processed.

	ERR
		- Used by server to identify an error in processing request.
		Error codes:
			0) Illegal operation.
			1) file not found.
			2) file already exists.

### Timing - sequence of exchange

	WRQ:
		->WRQ
		<-ACK
		->DATA, =512B :. more to come
		->DATA, <512B :. end
		<-ACK, :. write process completed successfully.

	RRQ:
		->RRQ
		<-ACK
		<-DATA, =512B :. more to come
		<-DATA, <512B :. end
		->ACK, :. read process completed successfully.

	CMD:
		->CMD
		<-DATA, =512B :. more to come
		<-DATA, <512B :. end
		or
		->CMD
		<-ACK, :. command executed successfully.

	ERR:
		->CMD
		<-ERR[0]
		or
		->CMD
		<-ERR[1]
		or
		->RRQ
		<-ERR[1]
		or
		->WRQ
		<-ERR[2]



### Use Case scenarios.

	server:
		./myftpd
		./myftpd /home/ftp/

	client:
		./myftp

	myftp client:
		% pwd
		% lpwd
		% dir
		% ldir
		% cd
		% lcd
		% <command to send RRQ> //eg get [filename]
		% <command to send WRQ> //eg put [filename]

