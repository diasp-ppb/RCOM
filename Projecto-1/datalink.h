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
#include <errno.h>


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
#define RR_0PACK 3
#define RR_1PACK 4
#define REJ_0PACK 5
#define REJ_1PACK 6

#define DATA_PACK 1
#define START_PACK 2
#define END_PACK 3

#define TRAMA_SIZE 200

struct datalinkINFO{
  int fd;
  char* port;
  struct termios oldtio;
  struct termios newtio;
};

char * createSet();
int sendMensage(int fd, char *message, int length);
int createAndSendPackage(int fd, int type);
int receiveFlag(int fd);
int receiveA(int fd, char *ch);
int receiveC(int fd, char *ch);
int checkBCC(int fd, char A, char C);
int receiveSupervision(int fd, char * C);
int checkRR_Reject(int C, unsigned char ch);

void installAlarm();
void atende();

int llopenTransmitter(int fd);
int llopenReceiver(int fd);

int llcloseTransmitter(int fd);
int llcloseReceiver(int fd);

int llwrite(char *buffer, int length,int C);
int llread(char *buffer, int C);

int stuffing(char * package, int length);
int deStuffing(char * package, int length);

int packagePayLoad(int C, int size, char * payload);
int createStart(char *filename, int length, unsigned int size, int type,char *package);

int getTrama (int fd, char* trama);
int extractPackage(char *package, char *trama,int length);

char makeBCC2(char* message, int length);

int openPort(char * port);
int closePort();
