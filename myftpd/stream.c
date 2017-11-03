/**
 * AUTHOR: Clem Davies, Daniel Kearsley
 * DATE: 3/11/17
 * FILENAME: stream.h
 * DESCRIPTION: Functions used to read and write data on a socket.
 */
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include "stream.h"

/*
 * Loops over a write to socket until entire nbytes is written from buf.
 */
int write_nbytes(int sd, char *buf, int nbytes)
{
	int nw = 0;
	int n = 0;
	for (n=0; n < nbytes; n += nw) {
		if ((nw = write(sd, buf+n, nbytes-n)) <= 0)
			return (nw); /* write error */
	}
  return n;
}

/*
 * Loops over a read from socket until entire nbytes read into buf.
 */
int read_nbytes(int sd, char *buf, int nbytes)
{
	int nr = 1;
	int n = 0;
	for (n=0; (n < nbytes) && (nr > 0); n += nr) {
		if ((nr = read(sd, buf+n, nbytes-n)) < 0){
			return (nr); /* read error */
		}
	}
	return (n);
}

/*
 * Writes a one byte char on the socket.
 */
int write_code(int sd, char code)
{
	/* write 1 byte code to socket */
	if (write(sd, (char*)&code, 1) != 1) return (-1);

	return 1;
}

/*
 * Reads a one byte char off the socket.
 */
int read_code(int sd, char* code)
{
	char data;

	/* read 1 byte code from socket */
	if(read(sd,(char *) &data,1) != 1) return -1;

	*code = data;

	return 1;
}

/*
 * Writes a two byte integer on the socket.
 */
int write_twobytelength(int sd, int length)
{
	short data = length;
	data = htons(data);  /* convert to network byte order */

	if (write(sd,&data, 2) != 2) return (-1);

	return 1;
}

/*
 * Reads a two byte integer off the socket.
 */
int read_twobytelength(int sd, int *length)
{
	short data = 0;

  if (read(sd, &data, 2) != 2) return (-1);

  short conv = ntohs(data); /* convert to host byte order */
  int t = (int)conv;
  *length = t;

	return 1;
}

/*
 * Writes a four byte integer on the socket.
 */
int write_fourbytelength(int sd, int length)
{
	int data = htonl(length); /* convert to network byte order */

	if (write(sd,&data, 4) != 4) return (-1);

	return 1;
}

/*
 * Reads a four byte integer off the socket.
 */
int read_fourbytelength(int sd, int *length)
{
	int data = 0;

  if (read(sd, &data, 4) != 4) return (-1);

  int conv = ntohl(data); /* convert to host byte order */
  *length = conv;

	return 1;
}
