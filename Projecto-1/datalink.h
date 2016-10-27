char * createSet();
int sendMensage(int fd, char *message, int length);
int createAndSendPackage(int fd, int type);
int receiveFlag(int fd);
int receiveA(int fd, char *ch);
int receiveC(int fd, char *ch);
int checkBCC(int fd, char A, char C);
int receiveSupervision(int fd, char * C);
int checkRR_Reject(int C, char ch);

void installAlarm();
void atende();

int llopenTransmitter(int fd);
int llopenReceiver(int fd);

int llcloseTransmitter(int fd);
int llcloseReceiver(int fd);

int llwrite(int fd, char *buffer, int length,int C);
int llread(int fd, char *buffer, int C);
int llstart(int fd, char* filename, int length, unsigned int size, int type);

int stuffing(char * package, int length);
int deStuffing(char * package, int length);

int packagePayLoad(int C, int size, char * payload);
int createStart(char *filename, int length, unsigned int size, int type,char *package);

int getTrama (int fd, char* trama);
int extractPackage(char *package, char *trama,int length);

char makeBCC2(char* message, int length);
