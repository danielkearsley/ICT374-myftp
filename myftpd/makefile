#makefile for server
myftpd: myftpd.o
	gcc myftpd.o -o myftpd
myftpd.o: myftpd.c
	gcc -c myftpd.c -o myftpd.o
clean:
	rm -f *.o myftpd