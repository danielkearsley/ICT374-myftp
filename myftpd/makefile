#makefile for server
myftpd: myftpd.o stream.o
	gcc myftpd.o stream.o -o myftpd
myftpd.o: myftpd.c
	gcc -c myftpd.c -o myftpd.o
stream.o: stream.c stream.h
	gcc -c stream.c -o stream.o
clean:
	rm -f *.o myftpd