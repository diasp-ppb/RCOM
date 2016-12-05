#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>


typedef struct ftp {
  int fd_socket; // o file descriptor do control socket
  int fd_data; // o  file descriptor do data socket
} ftp;



static int connectSocket(const char * IP,int PORT);
int FTPdownload(char * filename, ftp *ftp);
