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
#include "alarme.c"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define SEND 0
#define RECEIVE 1

volatile int STOP=FALSE;

char * createSet();
int sendMensage(int fd, char *message, int length);
int createAndSendSet(int fd);

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

    fd = open(argv[1], O_RDWR | O_NOCTTY );
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
/*	int state = 0;
	switch(state)
{
	case SEND:
		createAndSendSet(fd);
	break;
	case RECEIVE:
		
	break;
}*/
	createAndSendSet(fd);
	
	

	sleep(2);

	//installAlarm();
/*	gets(buf);
    
	int length = strlen(buf);

	printf("tamanho: %d \n", length+1);
	res = 0;
	
	while(!res){
	res=write(fd, buf, length+1);
	}*/


/*	 i = 0;
    int total = 0;
    while (STOP==FALSE) 
    {       /* loop for input */
/*      res = read(fd, &c, sizeof(char));    
      buf[i] = c;
      total += res;
      if ('\0' == c) 
		STOP=TRUE;
      i++;
    }


   printf("Received %d bytes: %s\n", total, buf);
	



    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
	*/
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


