# ICT374-myftp
Murdoch University group project for ICT374. Implement an FTP client and server in C on linux.

Designed and implemented by:
Clem
Daniel


## MYFTP protocol spec

### Syntax - formats of packets

	RRQ, WRQ
		| opcode | filename | 0 | mode | 0 |
	DATA
		| opcode | block num | opcode |
	ERROR
		| opcode | error code | error msg | 0 |


### Semantics - packet types, error codes

	RRQ
	WRQ
	DATA
	ERROR

	Error codes:
	0) not defined, see error message
	1) file not found
	3) file already exists

### Timing - sequence of exchange

	Write request:
		->WRQ
		->DATA, =512B :. more to come
		->DATA, <512B :. end

	Read request:
		->RRQ
		<-DATA, =512B :. more to come
		<-DATA, <512B :. end

	Error:
		->RRQ
		<-ERROR
		or
		->WRQ
		<-ERROR
		or
		->WRQ
		->DATA
		<-ERROR
