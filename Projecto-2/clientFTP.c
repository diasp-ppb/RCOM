#include "clientFTP.h"
#include "getip.c"



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



int validURL(char * url,unsigned int size){

regex_t regularExpression;


int retReg = regcomp(&regularExpression,"ftp://([a-zA-Z0-9])*:([a-zA-Z0-9])*+@+[a-zA-Z0-9]*+/+[a-zA-Z0-9._~@]*",REG_EXTENDED);

if(retReg != 0){
  printf("Fail: Couldnt compile regular expression\n");
  return 1;
}

int result;
if(!(result = regexec(&regularExpression,url,0,NULL,0))){
return 0;
}
else if(result = REG_NOMATCH){
  printf("Fail: Invalid URL \n");
  return 1;
}
else{
  printf("Fail: Cant validate URL\n");
  return 1;
}

return 0;
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

int parseHost(char *link, char *host){
    char tmpHost[32];
    int i = 0;
    int dataF = 0;
    while(*link != '\0'){
        if(*link == '[' || *link == ']')
            ++dataF;

        link++;
        if(dataF == 2){
            if(*link == '/')
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

int main(int argc, char** argv)
{
    if(argc != 2){
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
    free(ip);

    connectSocket(ip, 21);


    return 0;
}
