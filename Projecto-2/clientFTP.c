#include "clientFTP.h"
#include "getip.c"

static int connectSocket(const char *IP, int PORT) {
  int sockfd;
  struct sockaddr_in server_addr;

  /*server address handling*/
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      inet_addr(IP); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port =
      htons(PORT); /*server TCP port must be network byte ordered */

  /*open an TCP socket*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(0);
  }

  /*connect to the server*/
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
    perror("connect()");
    exit(0);
  }

  return sockfd;
}

int validURL(char *url, unsigned int size) {

  regex_t regularExpression;

  int retReg = regcomp(
      &regularExpression,
      "ftp://([a-zA-Z0-9])*:([a-zA-Z0-9])*+@+[a-zA-Z0-9]*+/+[a-zA-Z0-9._~@]*",
      REG_EXTENDED);

  if (retReg != 0) {
    printf("Fail: Couldnt compile regular expression\n");
    return 1;
  }

  int result;
  if (!(result = regexec(&regularExpression, url, 0, NULL, 0))) {
    return 0;
  } else if (result == REG_NOMATCH) {
    printf("Fail: Invalid URL \n");
    return 1;
  } else {
    printf("Fail: Cant validate URL\n");
    return 1;
  }

  return 0;
}

int FTPdownload(char *filename, ftp *ftp) {
  FILE *file;

  if (!(file = fopen(filename, "w"))) {
    printf("Unable to open file.\n");
    return 1;
  }
  int bytesRead;
  int bytesWrited;
  int totalBytesWrited = 0;

  char buf[2048]; // TODO  not sure what size should be

  while ((bytesRead = read(ftp->fd_data, buf, sizeof(buf)))) {

    if (bytesRead < 0) {
      printf("Fail: Nothing to read\n");
      return 1;
    }

    if ((bytesWrited = fwrite(buf, bytesRead, 1, file))) {
      printf("Fail: write in file\n");
      return 1;
    }

    totalBytesWrited += bytesWrited;
  }

  fclose(file);

  close(ftp->fd_data);

  return 0;
}

// TODO FALTA FAZER VERIFICAÇOES e ver se recebe ( 1* -> abrir data socket???
// how knows)
int FTPconnect(ftp *FTP, char *ip, int port) {

  int socket_fd;

  if ((socket_fd = connectSocket(ip, port)) < 0) {
    printf("Fail: Socket Connect \n");
    return 1;
  };

  FTP->fd_socket = socket_fd;
  FTP->fd_data = 0;

  // 1*
  return 0;
}

// receber mesagem socket control
int FTPsend(ftp *FTP, char *msg, int size) {

  int bytes;

  if ((bytes = write(FTP->fd_socket, msg, size)) < 0) {
    printf("Warning mensage wasnt read\n");
    return 1;
  }
  if (bytes != size) {
    printf("Warning the whole mensage wasn't sent \n");
    return 1;
  }
  return 0;
}

// maddar mensagem pro socket control
int FTPread(ftp *FTP, char *msg, unsigned int size) {
  FILE *sock = fdopen(FTP->fd_socket, "r");

  memset(msg,0,size);

  do{
    msg = fgets(msg, size, sock);
    printf("READ %s\n",msg );
  }while((!('1' <= msg[0] && msg[0] <= '5' ))|| msg[3] != ' ');


  return 0;
}


int FTPdisconnect(ftp * FTP) {

  char msg[2048];

  if(FTPread(FTP,msg,sizeof(msg))){
    printf("Fail: canno disconetc account\n");
    return 1;
  }

  sprintf(msg, "QUIT\r\n");

  if(FTPsend(FTP,msg,strlen(msg))){
    printf("Fail: cannnot send QUIT\n" );
    return 1;
  }


  close(FTP->fd_socket);


  return 0;

 }

int parseHost(char *link, char *host) {
  char tmpHost[32];
  int i = 0;
  int dataF = 0;
  while (*link != '\0') {
    if (*link == '[' || *link == ']')
      ++dataF;

    link++;
    if (dataF == 2) {
      if (*link == '/')
        break;

      tmpHost[i] = *link;
      i++;
    }
  }
  strcpy(host, tmpHost);
  printf("host: %s\n", host);
  if (dataF == 2)
    return 0;
  return 1;
}

int FTPlogin(ftp* FTP, char *user, char *pass) {

  char msg[2048];

  sprintf(msg,"USER %s \n",user);

  if(FTPsend(FTP,msg,strlen(msg))){
    printf("FAIL: Unable to send user name\n");
    return 1;
  }

  printf("%s\n",msg);

  if(FTPread(FTP,msg,sizeof(msg))){
    printf("FAIL: Unable to get response \n");
    return 1;
  }

  printf("%s\n",msg);

  memset(msg,0,sizeof(msg));

  sprintf(msg,"PASS %s \n",pass);
  if(FTPsend(FTP,msg,strlen(msg))){
    printf("FAIL: unable to send password \n");
    return 1;
  }


  if(FTPread(FTP,msg,sizeof(msg))){
    printf("FAIL: unable to get response password \n");
    return 1;
  }

      printf("%s\n",msg);
  return 0;
}

int FTPpasv(ftp * FTP){


  char msg[2048] ;

  sprintf(msg,"PASV\r\n");


  if(FTPsend(FTP,msg,strlen(msg))){
    printf("FAIL: unable to send PASV \n");
    return 1;
  }

    printf("%s\n",msg);

  if(FTPread(FTP,msg,sizeof(msg))){

    printf("FAIL: unable to receive to enter in passive mode \n");
    return 1;
  }

      printf("%s\n",msg);

  unsigned int ip1,ip2,ip3,ip4;
  int port1, port2;


  if(sscanf(msg,"227 Entering Passive Mode(%d,%d,%d,%d,%d,%d)",&ip1,&ip2,&ip3,&ip4,&port1,&port2)){
    printf("FAIL: sscanf");
    return 1;
  }
  printf("F %s\n",msg);


  int port = port1*256 + port2;

  memset(msg, 0, sizeof(msg));

  if (sprintf(msg, "%d.%d.%d.%d", ip1, ip2, ip3, ip4) < 0) {
		printf("ERROR: ip address make error.\n");
		return 1;
	}

  printf("IP %s \n", msg);
  printf("PORT %d \n", port);

  if((FTP->fd_data = connectSocket(msg,port)) < 0){
    printf("FAIL: PASV connectSocket \n");
    return 1;
  }
  return 0;
}




int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./clientFTP [<user>:<password>@]<host>/<url-path>\n");
    exit(1);
  }

  char link[64];
  strcpy(link, argv[1]);

  char *ip = malloc(64);
  char *host = malloc(32);

  parseHost(link, host);

  getIP(host, ip);
  printf("IP: %s\n", ip);

  ftp FTP;
  char user[32] = "";
  char pass[32] = "";
  char filename[250] = "/pub/robots.txt";
  FTPconnect(&FTP, ip, 21);
  FTPlogin(&FTP, user, pass);
  FTPpasv(&FTP);
  FTPdownload(filename,&FTP);
  FTPdisconnect(&FTP);



  free(ip);
  free(host);
  return 0;
}
