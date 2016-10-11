/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4
#define COMPLETE 5


volatile int STOP=FALSE;

int receiveFlag(int fd);
int receiveA(int fd, char *ch);
int receiveC(int fd, char *ch);
int checkBCC(int fd, char A, char C);
int receivePackage(int fd);

int main(int argc, char** argv)
{
    int fd, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	receivePackage(fd);

/*
    int i = 0;
    char ch = 'a';
    int total = 0;
    while (STOP==FALSE) 
    {       // loop for input 
      res = read(fd, &ch, 1);   // returns after 5 chars have been input 
      if(res <= 0)
	continue;
      buf[i] = ch;
      total += res;
      if ('\0' == ch) 
	STOP=TRUE;
      i++;
    }


   printf("Received %d bytes: %s\n", total, buf);
  
    //O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  

    int k = 0;
    int totalsent = 0;
    for(k = 0; k < total; k++)
    {  		
	char c = buf[k];
    	totalsent += write(fd,&c,sizeof(char));
    }


    printf("Sent %d bytes: %s\n", totalsent, buf);*/
sleep(2);


    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

char* createUA()
{
	char* UA = malloc(5*sizeof(char));
	UA[0] = F_FLAG;
	UA[1] = A_EM;
	UA[2] = C_UA;
	UA[3] = A_EM ^ C_UA;
	UA[4] = F_FLAG;

	return UA;
}

int receivePackage(int fd)
{
	int status = 0;
	int res;
	char A, C;
	do
	{
		switch(status)
		{
			case START:
			status = receiveFlag(fd);
			break;
			case FLAG_RCV:
			status = receiveA(fd, &A);
			break;
			case A_RCV:
			status = receiveC(fd, &C);
			break;
			case C_RCV:
			status = checkBCC(fd, A, C);
			break;
			case BCC_RCV:
			res = receiveFlag(fd);
			if(res == FLAG_RCV)
 			{
				printf("BCC Checked successfully\n");
				status = COMPLETE;
			}
			break;
		}
	}while(status != COMPLETE);

	return 0;
}

int receiveFlag(int fd)
{
	char ch;
	read(fd, &ch, 1);

	if(ch == F_FLAG)
		return A_RCV;
	else
		return FLAG_RCV;
}

int receiveA(int fd, char* ch)
{
	int res;
	res = read(fd, ch, 1);
	if(res <= 0)
		return START;
	else if(*ch == F_FLAG)
		return F_FLAG;
	return C_RCV;
}

int receiveC(int fd, char* ch)
{
	int res;
	res = read(fd, ch, 1);
	if(res <= 0)
		return START;
	else if(*ch == F_FLAG)
		return F_FLAG;
	return BCC_RCV;
}

int checkBCC(int fd, char A, char C)
{
	char ch;
	read(fd, &ch, 1);
	char expected = A ^ C;
	if(ch == expected)
		return COMPLETE;
	else
		return -1;
}


