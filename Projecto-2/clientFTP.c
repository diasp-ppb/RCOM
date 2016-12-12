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

 char* anonymous = "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";
 char* loggInfo = "ftp://([([A-Za-z0-9])*:([A-Za-z0-9])*@])*([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";


int validURL(char *url, unsigned int size) {


    if(size < 7)
    {
        printf("invalid URL size - %s - %d \n", url, size);
        return 1;
    }  

    regex_t regularExpression;

    int match  = sizeof(url);
    regmatch_t pmatch[match];


    char * selectExpression;

    int userMode;

    if(url[6] == '[')
    {
        selectExpression = loggInfo;
        userMode = 2;
    }
    else
    {
        selectExpression = anonymous;
        userMode = 3;
    }
    int retReg = regcomp(
        &regularExpression,
        selectExpression,
        REG_EXTENDED);

    if (retReg != 0) {
        printf("Fail: Couldnt compile regular expression\n");
        return 1;
    }

    int result;
    if (0 != (result = regexec(&regularExpression, url ,match , pmatch, REG_EXTENDED))) {
        printf("URL coundnt execute \n");   
        printf("%d",result);
        return 1;
    } 
    return userMode;
}

int FTPdownload(char *path, char *filename, ftp *ftp) {
    FILE *file;

    if (!(file = fopen(filename, "w"))) {
        printf("Unable to open file.\n");
        return 1;
    }
    int bytesRead;
    int bytesWrote;
    int totalBytesWrote = 0;

    char reqMsg[128];
    char response[128];
    sprintf(reqMsg,  "CWD %s\n", path);
    FTPsend(ftp, reqMsg, strlen(reqMsg));
    FTPread(ftp, response, sizeof(response));


    sprintf(reqMsg, "RETR %s\n", filename);
    FTPsend(ftp, reqMsg, strlen(reqMsg));
    FTPread(ftp, response, sizeof(response));

    char buf[2048]; // TODO  not sure what size should be

    while ((bytesRead = read(ftp->fd_data, buf, sizeof(buf)))) {
        if (bytesRead < 0) {
            printf("Fail: Nothing to read\n");
            return 1;
        }

        if (!(bytesWrote = fwrite(buf, bytesRead, 1, file))) {
            printf("Fail: writen in file\n");
            return 1;
        }

        totalBytesWrote += bytesWrote;
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

    char msg[2048];
    if(FTPread(FTP, msg, sizeof(msg)) != 0)
    {
        printf("error reading after connect\n");
        return -1;
    }
    //printf("Connect message: %s\n", msg);
    // 1*
    return 0;
}

// receber mesagem socket control
int FTPsend(ftp *FTP, char *msg, int size) {

    int bytes;
    printf("SEND: %s\n", msg);
    if ((bytes = write(FTP->fd_socket, msg, size)) <= 0) {
        printf("Warning mensage wasnt sent\n");
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

int parseLink(char *link, char *host, char *path, char *file) {
    char tmpHost[64];
    char tmpPath[64];
    char tmpFile[64];
    int i = 0;
    int dataF = 0;

    if(link[6] != '[') //finds if there is a user and a password on link
    {
        dataF = 2;
        link += 5;
    }

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
    tmpHost[i] = '\0';

    i = 0;
    char *startPath = link;
    int last = 0;
    while(link[i] != '\0'){
        if(link[i] == '/')
            last = i;
        i++;
    }

    for(i = 0; i <= last; i++)
        tmpPath[i] = startPath[i];
    tmpPath[i] = '\0';

    int j = 0;
    while(link[i] != '\0'){
        tmpFile[j] = link[i];
        i++;
        j++;
    }
    tmpFile[j] = '\0';



    strcpy(host, tmpHost);
    strcpy(path, tmpPath);
    strcpy(file, tmpFile);
    if (dataF == 2)
        return 0;
    return 1;
}

int FTPlogin(ftp* FTP, char *user, char *pass) {

    char msg[2048];

    sprintf(msg,"USER %s\n",user);

    if(FTPsend(FTP,msg,strlen(msg))){
        printf("FAIL: Unable to send user name\n");
        return 1;
    }

    //printf("SEND  USER:   %s\n",msg);

    if(FTPread(FTP,msg,sizeof(msg))){
        printf("FAIL: Unable to get response \n");
        return 1;
    }

        //  printf("READ USER response:   %s\n",msg);

    memset(msg,0,sizeof(msg));

    sprintf(msg,"PASS %s\n",pass);
    if(FTPsend(FTP,msg,strlen(msg))){
        printf("FAIL: unable to send password \n");
        return 1;
    }
    //printf("SEND PASS: %s\n", pass);


    if(FTPread(FTP,msg,sizeof(msg))){
        printf("FAIL: unable to get response password \n");
        return 1;
    }

    //printf("%s\n",msg);
    return 0;
}

int FTPpasv(ftp * FTP){
    char msg[2048] ;

    sprintf(msg,"PASV\n");

    if(FTPsend(FTP,msg,strlen(msg))){
        printf("FAIL: unable to send PASV \n");
        return 1;
    }

//    printf("%s\n",msg);

    if(FTPread(FTP,msg,sizeof(msg))){

        printf("FAIL: unable to receive to enter in passive mode \n");
        return 1;
    }

    //printf("PASV resp:\n%s\n",msg);

    unsigned int ip1,ip2,ip3,ip4;
    int port1, port2;

    // 227 Entering Passive Mode (90,130,70,73,91,233).
    if(sscanf(msg,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",&ip1,&ip2,&ip3,&ip4,&port1,&port2) < 0){
        printf("FAIL: sscanf\n");
        return 1;
    }


    int port = port1*256 + port2;

    memset(msg, 0, sizeof(msg));

    if (sprintf(msg, "%d.%d.%d.%d", ip1, ip2, ip3, ip4) < 0) {
        printf("ERROR: ip address make error.\n");
        return 1;
    }

    //printf("IP %s \n", msg);
    //printf("PORT %d \n", port);

    if((FTP->fd_data = connectSocket(msg,port)) < 0){
        printf("FAIL: PASV connectSocket \n");
        return 1;
    }
    return 0;
}



int parseNameAndPass(char *link,char * user, char * pass){
    int len = strlen(link);
    int index = 0;
    while( index < len && link[index] != '[')
    {
        index++;
    } 

    char tempUser[64];
    char tempPass[64];

    index++;
    int i = 0;
    while(index < len && link[index] != ':' && i < 64)
    {
        tempUser[i] = link[index];
        index++;
        i++;
    }

    tempUser[i] = '\0';

    index++;
    i=0;
    while(index < len && link[index] != '@' && i < 64)
    {
        tempPass[i] = link[index];
        index++;
        i++;
    }

    tempPass[i] = '\0';
    
    strcpy(user,tempUser);
    strcpy(pass,tempPass);

    printf("%s \n",tempUser);
    printf("%s \n",tempPass);


    return 0;
} 


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: ./clientFTP ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    int mode;
    if((mode = validURL(argv[1], strlen(argv[1]))) < 2) {
         exit(1);
    }



    
    

    char link[128];
    strcpy(link, argv[1]);

    char *ip = malloc(64);
    char *host = malloc(64);
    char *path = malloc(64);
    char *file = malloc(64);

    parseLink(link, host, path, file);


    getIP(host, ip);




    printf("IP: %s\n", ip);
    printf("host: %s\n", host);
    printf("path: %s\n", path);
    printf("file: %s\n", file);


    char * user;
    char * pass;


    //anonymous 
    if(mode == 3){

    user = "anonymous";
    pass = "up2014@fe.up.pt";

    }
    else if(mode == 2){
    
    user = malloc(sizeof(char)*64);
    pass = malloc(sizeof(char)*64);

    parseNameAndPass(link,user,pass);


    }

    
    ftp FTP;

    FTPconnect(&FTP, ip, 21);
    FTPlogin(&FTP, user, pass);
    FTPpasv(&FTP);
    FTPdownload(path, file, &FTP);
    FTPdisconnect(&FTP);


    free(ip);
    free(host);
    free(path);
    free(file);
    return 0;
}
