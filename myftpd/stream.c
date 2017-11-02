/*
 *  stream.c  -	(Topic 11, HX 22/5/1995)
 *	 	routines for stream read and write.
 */

#include  <sys/types.h>
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include  "stream.h"

#include <unistd.h>
#include <stdio.h>




int writen(int sd, char *buf, int nbytes)
{
    short data_size = nbytes;     /* short must be two bytes long */
    int n, nw;

    if (nbytes > MAX_BLOCK_SIZE)
         return (-3);    /* too many bytes to send in one go */

    /* send the data size */
    data_size = htons(data_size);
    if (write(sd, (char *) &data_size, 1) != 1) return (-1);
    if (write(sd, (char *) (&data_size)+1, 1) != 1) return (-1);

    /* send nbytes */
    for (n=0; n<nbytes; n += nw) {
         if ((nw = write(sd, buf+n, nbytes-n)) <= 0)
             return (nw);    /* write error */
    }
    return (n);
}


int readn(int sd, char *buf, int bufsize)
{
    short data_size;    /* sizeof (short) must be 2 */
    int n, nr, len;

    /* check buffer size len */
    if (bufsize < MAX_BLOCK_SIZE)
         return (-3);     /* buffer too small */

    /* get the size of data sent to me */
    if (read(sd, (char *) &data_size, 1) != 1) return (-1);
    if (read(sd, (char *) (&data_size)+1, 1) != 1) return (-1);
    len = (int) ntohs(data_size);  /* convert to host byte order */

    /* read len number of bytes to buf */
    for (n=0; n < len; n += nr) {
        if ((nr = read(sd, buf+n, len-n)) <= 0)
            return (nr);       /* error in reading */
    }
    return (len);
}




int write_nbytes(int sd, char *buf, int nbytes)
{
	int nw = 0;
	int n = 0;
	for (n=0; n < nbytes; n += nw) {
		if ((nw = write(sd, buf+n, nbytes-n)) <= 0)
			return (nw);    /* write error */
	}
  return n;
}


int read_nbytes(int sd, char *buf, int nbytes)
{
	int nr = 1;
	int n = 0;
	for (n=0; (n < nbytes) && (nr > 0); n += nr) {
		if ((nr = read(sd, buf+n, nbytes-n)) < 0){
			return (nr);       /* error in reading */
		}
	}
	return (n);
}


int write_code(int sd, char code)
{

	/* send the code */
	if (write(sd, (char*)&code, 1) != 1) return (-1);

	//return success
	return 1;
}

int read_code(int sd, char* code)
{
	char data;

	// read 1 byte code from socket
	if(read(sd,(char *) &data,1) != 1)
		return -1; // read failed
	*code = data;
	// return success;
	return 1;
}


int write_twobytelength(int sd, int length)
{
	short data = length;
	data = htons(data);  //convert to network byte order

	if (write(sd,&data, 2) != 2) return (-1);

	return 1;
}

int read_twobytelength(int sd, int *length)
{
	short data = 0;

  if (read(sd, &data, 2) != 2) return (-1);
  short conv = ntohs(data); /* convert to host byte order */
  int t = (int)conv;
  *length = t;

	return 1;
}

int write_fourbytelength(int sd, int length)
{
	int data = htonl(length); //convert to network byte order

	if (write(sd,&data, 4) != 4) return (-1);

	return 1;
}

int read_fourbytelength(int sd, int *length)
{
	int data = 0;

  if (read(sd, &data, 4) != 4) return (-1);
  int conv = ntohl(data); /* convert to host byte order */
  *length = conv;

	return 1;
}


