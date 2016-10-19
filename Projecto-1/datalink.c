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

#define TRANSMITTER 0
#define RECEIVER 1

#define SET_PACK 0
#define UA_PACK 1
#define DISC_PACK 2

#define DATA_PACK 1
#define START_PACK 2
#define END_PACK 3


volatile int STOP=FALSE;

char * createSet();
int sendMensage(int fd, char *message, int length);
int createAndSendPackage(int fd, int type);
int receiveFlag(int fd);
int receiveA(int fd, char *ch);
int receiveC(int fd, char *ch);
int checkBCC(int fd, char A, char C);
int receiveSupervision(int fd);


volatile int flag=1, conta=1;
void installAlarm();
void atende();

int llopenTransmitter(int fd);
int llopenReceiver(int fd);

int llcloseTransmitter(int fd);
int llcloseReceiver(int fd);

int llwrite(int fd, char *buffer, int length);
int llread(int fd,char *buffer);

int stuffing(unsigned char * frame, int length);
int deStuffing(unsigned char * frame, int length);

int packagePayLoad(int C, int size, char * payload);
int createSendStartEnd(int fd, char *filename, int length, unsigned int size, int type);

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}

void installAlarm()
{
  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
 printf("Alarme Instalado\n");
}


//TODO: ver argumento porta pwp18
int llopen(int flag, int fd)
{
  if(flag == TRANSMITTER)
    llopenTransmitter(fd);
  else if(flag == RECEIVER)
    llopenReceiver(fd);
  return 1;
}

int llclose(int flag, int fd)
{
	if(flag == TRANSMITTER)
		llcloseTransmitter(fd);
	else if(flag == RECEIVER)
		llcloseReceiver(fd);
		sleep(5);
		return 1;
}


int llwrite(int fd, char *buffer, int length){
	createSendStartEnd(fd, "giro", 4, length, START);
	return 0;
}

int llread(int fd,char *buffer){
	int i = 0;
	while(read(fd, &buffer[i], 1)){}
	return 0;
}

int main(int argc, char** argv)
{
  int fd;
  struct termios oldtio,newtio;

  if ( (argc < 3) ||
       ((strcmp("/dev/ttyS0", argv[1])!=0) &&
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0  RECEIVER||TRANSMITTER\n");
    exit(1);
  }

  if( (strcmp("RECEIVER", argv[2]) != 0) && (strcmp("TRANSMITTER", argv[2]) != 0))
  {
     printf("choose RECEIVER or TRANSMITTER\n");
     exit(2);
  }

  fd = open(argv[1], O_RDWR | O_NOCTTY|O_NONBLOCK);
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

  int mode = 0;
  if(strcmp("RECEIVER", argv[2]) == 0)
     mode = RECEIVER;
  else
     mode = TRANSMITTER;

 /*
  llopen(mode, fd);

	llclose(mode, fd);
*/
	if(mode == TRANSMITTER){
	char *test = malloc(2);
	test[0] = 'a';
	test[1] = 'b';
	llwrite(fd, test, 2);
	free(test);
	}
	else if(mode == RECEIVER){
	char* test = malloc(20);
	llread(fd, test);
	int t;
	for(t= 0; t < 20; t++){
		printf("t: %x \n",test[t]);
	}
	free(test);
	}

	unsigned char *jesus = malloc(5);
	jesus [0] = 0xF4;
	jesus [1] = 0x7E;
	jesus [2] = 0x7D;
	jesus [3] = 0x55;
	jesus [4] = 0x7D;
	 int l;
	 printf("PRE Stuffing\n");
  l= stuffing(jesus,5);

	printf("Stuffing: SIZE: %d \n",l);

	int t;
	for(t= 0; t < l; t++){
		printf("t: %x \n",jesus[t]);
	}

	l = deStuffing(jesus,l);


printf("deStuffing: SIZE: %d \n",l);

	for(t= 0; t < l; t++){
		printf("t: %x \n",jesus[t]);
	}

	free(jesus);

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}

int llopenTransmitter(int fd)
{
  int noResponse = 1;

	installAlarm();

 	while(conta < 4 && noResponse != COMPLETE)
  {
	   if(flag)
     {
       alarm(3);
       flag=0;
        createAndSendPackage(fd,SET_PACK);
     }
     noResponse = receiveSupervision(fd);
	}
  return 0;
}

int llopenReceiver(int fd)
{
  flag = 0; //To dont break processing package loop
  if(receiveSupervision(fd) == COMPLETE)
    createAndSendPackage(fd,UA_PACK);

  return 0;
}

int llcloseTransmitter(int fd)
{
	int noResponse = 1;

	installAlarm();

 	while(conta < 4 && noResponse != COMPLETE)
  {
	   if(flag)
     {
       alarm(3);
       flag=0;
        createAndSendPackage(fd,DISC_PACK);
     }
     noResponse = receiveSupervision(fd);
		 if(noResponse == COMPLETE)
		 printf("received DISC \n");
	}

int i;
for(i= 0; i < 3; i++){
	createAndSendPackage(fd,UA_PACK);//TODO NEED FIX SHOULD ONLY SEND ONCE
}



	printf("sent UA \n");
  return 0;
}

int llcloseReceiver(int fd)
{
	flag = 0; //To dont break processing package loop
  if(receiveSupervision(fd) == COMPLETE)
    createAndSendPackage(fd,DISC_PACK);

	if(receiveSupervision(fd) == COMPLETE)
	{
		printf("received UA\n");
		return 0;
	}
  return 1;
}

int stuffing(unsigned char * frame, int length){

		int size = length;
		int i;
		for(i = 0; i < length;i++){
			char oct = frame[i];
			if(oct == F_FLAG || oct == ESC){
					size++;
			}
		}
		frame = realloc(frame, size);

		for(i = 0; i < size; i++){
		 char oct = frame[i];
		 if(oct == F_FLAG || oct == ESC){
			 	 printf("moving");
			 memmove(frame + i + 2, frame + i+1, size - i);
			 printf("moved");
			 if (oct == F_FLAG) {
			 		frame[i+1] = XOR_7E_20;
					frame[i] = ESC ;
			 }
			 else frame[i+1] = XOR_7D_20;
		 }
		}
		return size;
}
int deStuffing(unsigned char * frame, int length){
	int size = length;

	int i;
	for(i = 0; i < size; i++){
		char oct = frame[i];
		if(oct == ESC)
		{
			if(frame[i+1] == XOR_7E_20){
			 frame[i] = F_FLAG;
			 memmove(frame + i + 1, frame + i + 2,length - i + 2);

		 }
			else if(frame[i+1] == XOR_7D_20){
				memmove(frame + i + 1, frame + i + 2,length - i + 2);
			}
			 size--;
		}
	}
	frame = realloc(frame,size);
	return size;
}

char  makeBCC2(char* message, int length){
	char bcc = 0;
	int i;
	for(i = 0; i < length; i++){
		bcc ^= message[i];
	}
	return bcc;
}
int receiveSupervision(int fd)
{
	int status = 0;
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
			status = receiveFlag(fd);
			if(status == FLAG_RCV)
				{status = COMPLETE;
				printf("full package!\n");}
			break;
		}

	}while(status != COMPLETE && flag == 0);
	//printf("Flag %d \n",flag);
	printf("status %d \n",status);
	return status;
}

int receiveFlag(int fd)
{
	//printf("antes FLAG \n");
	char ch;
	read(fd, &ch, 1);

	//printf("Flag value: %x \n",ch);
	if(ch == F_FLAG)
		return FLAG_RCV;
	else
		return START;
}

int receiveA(int fd, char* ch)
{
	int res;
	res = read(fd, ch, 1);
	//printf("readA %x \n",*ch);
	if(res <= 0)
		return START;
	else if(*ch == F_FLAG)
		return FLAG_RCV;
	return A_RCV;
}

int receiveC(int fd, char* ch)
{

	int res;
	res = read(fd, ch, 1);
	//printf("receiveC %x \n",*ch);
	if(res <= 0)
		return START;
	else if(*ch == F_FLAG)
		return FLAG_RCV;
	return C_RCV;
}

int checkBCC(int fd, char A, char C)
{
	char ch;
	read(fd, &ch, 1);
	char expected = A ^ C;

	if(ch == expected)
		return BCC_RCV;
	else
		return START;
}

char* createSet()
{
	char* set = malloc(5 * sizeof(char));

	set[0] = F_FLAG;
	set[1] = A_EM;
	set[2] = C_SET;
	set[3] = A_EM ^ C_SET;
	set[4] = F_FLAG;

return set;
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

char* createDisc()
{
	char* UA = malloc(5*sizeof(char));
	UA[0] = F_FLAG;
	UA[1] = A_EM;
	UA[2] = C_DISC;
	UA[3] = A_EM ^ C_UA;
	UA[4] = F_FLAG;

	return UA;
}

int createAndSendPackage(int fd,int type)
{
	char * msg;
	if(SET_PACK == type)
		msg = createSet();
	else if (UA_PACK == type)
		msg = createUA();
	else if(DISC_PACK == type)
		msg = createDisc();
 	int res = sendMensage(fd,msg,5);
	sleep(1);
	free(msg);
	return res;
}

int createSendStartEnd(int fd, char *filename, int length, unsigned int size, int type){
	char * start = malloc(length+10);
	start [0] = type;
	start [1] = 0;
	start [2] = 0x04;
	start [3] = size % 0x100;
	start [4] = start[3] % 0x100;
	start [5] = (size  % 0x10000) % 0x100;
	start [6] = size % 0x1000000;
	start [7] = 1;
	start [8] = length;

	int i;
	for ( i = 0; i < length; i++){
		start[9+i] = filename[i];
	}

	start[9 + length] = makeBCC2(start, length + 9);
	int lgt = packagePayLoad(0, (length + 10), start);
	sendMensage(fd, start, lgt);
	free(start);
	return 0; //TODO
}

int packagePayLoad(int C, int size, char * payload){
	char * buffer = malloc(size + 5);
	buffer[0] = F_FLAG;
	buffer[1] = A_EM;
	buffer[2] = C;
	buffer[3] = buffer[1] ^ buffer[2];
	int i;
	for(i = 0; i < size; i++)
	{
		buffer[i + 4] = payload[i];
	}

	buffer[3 + size] = F_FLAG;

	free(payload);
	payload = buffer;
	return (size + 5);
}
int sendMensage(int fd, char *message, int length)
{
	int res  = 0;
	while(res <= 0){
	res=write(fd, message, length);
	printf("sending...\n");
	}
  return 0;
}
