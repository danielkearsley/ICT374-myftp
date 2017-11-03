/**
 * AUTHOR: Clem Davies, Daniel Kearsley
 * DATE: 3/11/17
 * FILENAME: stream.h
 * DESCRIPTION: Defines functions used to read and write data on a socket.
 */

/*
 * Writes a stream of bytes on socket sd from buf
 * returns:
 * >0 : number of bytes written
 * otherwise : write error
 */
int write_nbytes(int sd, char *buf, int nbytes);

/*
 * Reads a stream of bytes on socket sd to buf
 * returns:
 * >0 : number of bytes read
 * otherwise : read error
 */
int read_nbytes(int sd, char *buf, int nbytes);

/*
 * Writes a one byte char from opcode to socket sd.
 * return: -1 : write failed
 *					1 : write success
 */
int write_code(int sd, char code);

/*
 * Reads a one byte char from socket sd to opcode.
 * return: -1 : read failed
 *					1 : read success
 */
int read_code(int sd,char *code);

/*
 * Writes a two byte integer from length to socket sd.
 * return: -1 : write failed
 *					1 : write success
 */
int write_twobytelength(int sd, int length);

/*
 * Reads a two byte integer from socket sd to length.
 * return: -1 : read failed
 *					1 : read success
 */
int read_twobytelength(int sd, int *length);

/*
 * Writes a four byte integer from length to socket sd.
 * return: -1 : write failed
 *					1 : write success
 */
int write_fourbytelength(int sd, int length);

/*
 * Reads a four byte integer from socket sd to length.
 * return: -1 : read failed
 *					1 : read success
 */
int read_fourbytelength(int sd, int *length);
