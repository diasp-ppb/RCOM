/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"
#include <signal.h>


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
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

char * createSet();
int sendMensage(int fd, char *message, int length);
int createAndSendSet(int fd);
int receiveFlag(int fd);
int receiveA(int fd, char *ch);
int receiveC(int fd, char *ch);
int checkBCC(int fd, char A, char C);
int receiveUA(int fd);


volatile int flag=1, conta=1;
void installAlarm();
void atende();

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


	int noResponse = 1;


	installAlarm();


 	while(conta < 4 && noResponse != COMPLETE){


	if(flag)
		{
			alarm(3);
			flag=0;
			createAndSendSet(fd);

		}

	noResponse = receiveUA(fd);


	}


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}

char* createSet(){
	char* set = malloc(5 * sizeof(char));

	set[0] = F_FLAG;
	set[1] = A_EM;
	set[2] = C_SET;
	set[3] = A_EM ^ C_SET;
	set[4] = F_FLAG;

return set;
}

int sendMensage(int fd, char *message, int length){
	int res  = 0;
	while(res <= 0){
	res=write(fd, message, length);
	}

return res;
}
int createAndSendSet(int fd){
	char * msg  = createSet();
 	int res = sendMensage(fd,msg,5);
	free(msg);
	return res;
}

int receiveUA(int fd)
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
	}while(status != COMPLETE && flag == 0);

	return status;
}


int receiveFlag(int fd)
{//printf("antes FLAG \n");
	char ch;
	read(fd, &ch, 1);

	//printf("%x \n",ch);
	if(ch == F_FLAG)
		return A_RCV;
	else
		return FLAG_RCV;
}

int receiveA(int fd, char* ch)
{
	int res;
	res = read(fd, ch, 1);
	//printf("%x \n",*ch);
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
	printf("%x \n",*ch);
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

printf("%x \n",expected);
	if(ch == expected)
		return COMPLETE;
	else
		return -1;
}




void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}


void installAlarm()
{
  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
  printf("Vou terminar.\n");
}
