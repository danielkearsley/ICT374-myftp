/*
 *  stream.c  -	(Topic 11, HX 22/5/1995)
 *	 	routines for stream read and write.
 */

#include  <sys/types.h>
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include  "stream.h"

#include <unistd.h>



int readn(int fd, char *buf, int bufsize)
{
    short data_size;    /* sizeof (short) must be 2 */
    int n, nr, len;

    /* check buffer size len */
    if (bufsize < MAX_BLOCK_SIZE)
         return (-3);     /* buffer too small */

    /* get the size of data sent to me */
    if (read(fd, (char *) &data_size, 1) != 1) return (-1);
    if (read(fd, (char *) (&data_size)+1, 1) != 1) return (-1);
    len = (int) ntohs(data_size);  /* convert to host byte order */

    /* read len number of bytes to buf */
    for (n=0; n < len; n += nr) {
        if ((nr = read(fd, buf+n, len-n)) <= 0)
            return (nr);       /* error in reading */
    }
    return (len);
}

int writen(int fd, char *buf, int nbytes)
{
    short data_size = nbytes;     /* short must be two bytes long */
    int n, nw;

    if (nbytes > MAX_BLOCK_SIZE)
         return (-3);    /* too many bytes to send in one go */

    /* send the data size */
    data_size = htons(data_size);
    if (write(fd, (char *) &data_size, 1) != 1) return (-1);
    if (write(fd, (char *) (&data_size)+1, 1) != 1) return (-1);

    /* send nbytes */
    for (n=0; n<nbytes; n += nw) {
         if ((nw = write(fd, buf+n, nbytes-n)) <= 0)
             return (nw);    /* write error */
    }
    return (n);
}


int read_code(int fd,char* opcode)
{
	char data;

	// read 1 byte opcode from socket
	if(read(fd,(char *) &data,1) != 1)
		return -1; // read failed
	*opcode = data;
	// return success;
	return 1;
}

int write_code(int fd, char opcode)
{

	/* send the opcode */
	if (write(fd, (char*)&opcode, 1) != 1) return (-1);

	//return success
	return 1;
}


int write_twobytelength(int fd, short length)
{
	short data = length;
	data = htons(data);
	if (write(fd, (char*)&data, 1) != 1) return (-1);
	if (write(fd, (char*)(&data)+1, 1) != 1) return (-1);

	return 1;
}

int read_twobytelength(int fd, int* length)
{
	short data;
  if (read(fd, (char *) &data, 1) != 1) return (-1);
  if (read(fd, (char *) (&data)+1, 1) != 1) return (-1);
  *length = (int) ntohs(data);  /* convert to host byte order */

	return 1;
}


int write_fourbytelength(int fd, int length)
{

	return 1;
}

int read_fourbytelength(int fd, int* length)
{

	return 1;
}


