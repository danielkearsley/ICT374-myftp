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
		-Client sends either a RRQ or WRQ with a filename and mode.
		-Server processes request and returns a DATA packet or ERR packet.
		-Server continues to send DATA packets until end of file.
	WRQ
		-Client sends a WRQ packet with filename and mode.
		-Server returns either an ACK packet or ERR packet back.
		-If ACK is received on client,
			client can begin to send DATA packets to server
			until no more DATA packets to send.
		-When server receives the last data packet it returns an ACK packet,
			identifying that file has been written to disk successfully.
			if write to disk unsuccessful
			return an ERR packet with code [3]
		Modes:
			0) ascii
			1) binary

	CMD
		-Client sends a CMD packet with one of the following commands.
		-Server processes request and if successful
			returns a DATA packet, ACK packet
			or if unsuccessful
			returns ERR packet with appropriate error code.

		Commands: cmd name, desc, response packet
			-PWD
				Display current directory of client
				Returns DATA packet with current client directory string.
			-LPWD
				Display current directory of server
				Returns DATA packet with current server directory string.
			-DIR
				Display current directory listing of client
				Returns DATA packet with current client directory listing.
			-LDIR
				Display current directory listing of server
				Returns DATA packet with current server directory listing.
			-CD [dirname]
				Changes current directory of client
				Returns ACK
			-LCD [dirname]
				Changes current directory of server
				Returns ACK
			-QUIT
				terminates myftp session
				Returns ACK

	DATA
		-Used by client and server to send chunked data of files to the other.
		-If payload size is 512B then there is more data to send.
		-If payload size is 0-511B then there is no more data to send.

	ACK
		-A 2B packet used to acknowledge request has been processed successfully.

	ERR
		-Used by server to identify an error in processing request.
		Error codes:
			0) Illegal operation.
			1) File not found.
			2) File already exists.
			3) Write to disk failed.

### Timing - sequence of exchange

	key:
		-> client sends packet to server
		<- server sends packet to client

	WRQ:
		->WRQ
		<-ACK
		->DATA, =512B :. more to come
		->DATA, <512B :. end
		<-ACK, :. write to disk successful

	RRQ:
		->RRQ
		<-ACK
		<-DATA, =512B :. more to come
		<-DATA, <512B :. end

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
		or
		->WRQ
		<-ACK
		->DATA
		->DATA
		<-ERR[3]
		or
		->RRQ
		<-DATA
		<-DATA
		->ERR[3]



### Use Case scenarios.

	Create a timeline of processing in both the client and the server for each command.

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
			- Files of size 100B, 512B, 10MB
			- Files in ascii and binary


