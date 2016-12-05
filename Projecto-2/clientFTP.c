#include "clientFTP.h"



static int connectSocket(const char * IP,int PORT){
  int	sockfd;
  struct	sockaddr_in server_addr;

  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IP);	/*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(PORT);		/*server TCP port must be network byte ordered */

  /*open an TCP socket*/
  if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket()");
          exit(0);
      }
  /*connect to the server*/
      if(connect(sockfd,
             (struct sockaddr *)&server_addr,
       sizeof(server_addr)) < 0){
          perror("connect()");
    exit(0);
  }

  return  sockfd;
}


int FTPdownload(char * filename, ftp *ftp){

  FILE * file;

  if(!(file = fopen(filename,"w"))){
    printf("Unable to open file.\n");
    return 1;
  }
int bytesRead;
int bytesWrited;
int totalBytesWrited = 0;

char buf[2048]; // TODO  not sure what size should be


while((bytesRead = read(ftp->fd_data,buf,sizeof(buf)))){

if(bytesRead < 0){
  printf("Fail: Nothing to read\n");
  return 1;
}

if((bytesWrited = fwrite(buf, bytesRead,1,file))){
  printf("Fail: write in file\n");
  return 1;
}

  totalBytesWrited += bytesWrited;
}

fclose(file);

close(ftp->fd_data);

return 0;
}


//TODO FALTA FAZER VERIFICAÇOES e ver se recebe ( 1* -> abrir data socket??? how knows)
int FTPconnect(ftp * FTP, char * ip, int port){

int socket_fd;

if((socket_fd = connectSocket(ip,port)) != 0){
  printf("Fail: Socket Connect \n");
  return 1;
};

FTP->fd_socket = socket_fd;
FTP->fd_data = 0;

//1*
return 0;
}


//receber mesagem socket control
int FTPsend(){
  return 0;
}

// maddar mensagem pro socket control
int FTPread(){
  return 0;
}

//TODO Mandar Disc pro servidor
// fechar socket
int FTPdisconnect(){


  return 0;
}
