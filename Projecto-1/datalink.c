#include "datalink.h"
volatile int flag=1, conta=1;
struct datalinkINFO dataINFO;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}

void installAlarm(){
	(void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
	printf("Alarme Instalado\n");
}


//TODO: ver argumento porta pwp18
int llopen(char *port, int flag)
{
	openPort(port);
	int status = 1;
	if(flag == TRANSMITTER)
		status = llopenTransmitter(dataINFO.fd);
	else if(flag == RECEIVER)
		status = llopenReceiver(dataINFO.fd);
	return status;
}

int llclose(int flag)
{
	if(flag == TRANSMITTER)
		llcloseTransmitter(dataINFO.fd);
	else if(flag == RECEIVER)
		llcloseReceiver(dataINFO.fd);

	closePort();
	return 1;
}


int llwrite(char *buffer, int length, int C){
/*	int i;
	printf("llwrite buffer : size:%d \n",length);
	for(i = 0; i < length ; i ++){
		printf("%d - %x\n",i, buffer[i]);
	}*/
	conta = 0;


	int fd = dataINFO.fd;

	char * copy = malloc((length + 1) * 2);
	memcpy(copy,buffer,length);


	char bcc2 = makeBCC2(buffer, length);
	copy[length] = bcc2;

	/*for(i = 0; i < length + 1 ; i ++){
		printf("%d - %x - %c\n",i, copy[i], copy[i]);
	}*/
	int size = stuffing(copy, length + 1);



	size = packagePayLoad(C, size, copy);



	//printf("llwrite trama I : length: %d -  size:%d \n", length, size);
/*	for(i = 0; i < size ; i ++){
		printf("%d - %x - %c\n",i, copy[i], copy[i]);
	}*/

	int noResponse = 1;
	char ch;
	int status = -1;
	flag = 1;
	conta = 0;

	while(conta < 4  && status != COMPLETE){
		if(flag){
			alarm(3);
			flag=0;
			sendMensage(fd,copy,size);
			printf("sendMensage! \n");
		}

		noResponse = receiveSupervision(fd,&ch);
		unsigned char cha = (unsigned char) ch;
	//	printf("cha %x C %d\n", cha ,C);

		if(noResponse == COMPLETE && (cha == C_RR0 || cha == C_RR1 || cha == C_REJ0 || cha == C_REJ1)){
			status = checkRR_Reject(C, cha);
			printf("status %d\n", status);
		}
	}

	conta =  0;
	alarm(0);

	free(copy);
	return status;
}

int llread(char *buffer, int C){
	int fd = dataINFO.fd;
	char *trama  = malloc(TRAMA_SIZE);
	int size  = getTrama(fd, trama);
	if(size < 5)
	{
		printf("Wrong trama: size: %d\n",size);
		if(C == 1)
		createAndSendPackage(fd, REJ_1PACK);
		else if(C == 0)
		createAndSendPackage(fd, REJ_0PACK);
		return -1;
	}

	printf("package Valid size, %d\n",size);

	int Ctrama = trama[2];
	size -= 5;
	char *package = malloc(TRAMA_SIZE);
	memmove(package, trama + 4, size);

	printf("package extracted:size %d \n",size);

	size = deStuffing(package, size);
/*
	int i;
	for(i = 0; i < size;i++){
		printf("p: %x \n",(unsigned char) package[i]);
	}*/

	char bcc = makeBCC2( package, size-1);

	if(bcc == package[size - 1])
	{
		printf("BCC2 check: %d\n", C);
		if(Ctrama == 1)
		createAndSendPackage(fd, RR_0PACK);
		else if (Ctrama == 0)
		createAndSendPackage(fd, RR_1PACK);

		if(C != Ctrama){
			printf("pacote repetido\n");
			return -1;
		}

	}
	else
	{
		printf("BCC2 fail\n");
		if(C == 1)
		createAndSendPackage(fd, REJ_1PACK);
		else if (C == 0)
		createAndSendPackage(fd, REJ_0PACK);

		return -1;
	}

	size -= 1; //removes bcc2;

	//buffer = realloc(buffer, size);

	memcpy(buffer, package,size);

	free(package);
/*
	for(i = 0; i < size; i++){
		printf("%d - %x - %c \n", i, buffer[i], buffer[i]);
	}*/

	return size;
}

int llopenTransmitter(int fd)
{
	installAlarm();
	int noResponse = 1;
	conta = 0;

	while(conta < 4 && noResponse != COMPLETE)
	{
		if(flag)
		{
			alarm(3);
			flag=0;
			createAndSendPackage(fd,SET_PACK);
		}
		char C;
		noResponse = receiveSupervision(fd,&C);
	}
	if(conta < 4)
	{
		printf("Connection opened with success\n");
		return 0;
	}
	printf("unable to connect to receiver\n");
	return 1;
}

int llopenReceiver(int fd)
{
	flag = 0; //To dont break processing package loop //TODO WUT???
	char C;
	printf("waiting for start pack\n");
	if(receiveSupervision(fd,&C) == COMPLETE) // TODO meter as flags
		createAndSendPackage(fd,UA_PACK);
	printf("connection opened successfuly\n");
	return 0;
}

int llcloseTransmitter(int fd)
{
	int noResponse = 1;
	char C;
	conta = 0;

	while(conta < 4 && noResponse != COMPLETE)
	{
		if(createAndSendPackage(fd, DISC_PACK) == 5)
		{
			alarm(3);
			flag = 0;

			while(flag == 0 && noResponse != COMPLETE)
			noResponse = receiveSupervision(fd, &C);

		}
	}

	if(conta < 4){
		createAndSendPackage(fd,UA_PACK);//TODO NEED FIX SHOULD ONLY SEND ONCE
		printf("connection closed sucessfully\n");
		return 0;
	}
	printf("no success closing connection\n");
	return 1;
}

int llcloseReceiver(int fd)
{
	flag = 0; //To dont break processing package loop
	char C;
	if(receiveSupervision(fd,&C) == COMPLETE)
	createAndSendPackage(fd,DISC_PACK);

	if(receiveSupervision(fd,&C) == COMPLETE)
	{
		printf("connection closed successfully\n");
		return 0;
	}
	return 1;
}

int stuffing(char * package, int length){

	int size = length;
	int i;
	for(i = 0; i < length;i++){
		char oct = package[i];
		if(oct == F_FLAG || oct == ESC){
			size++;
		}
	}

	if(size == length)
	return size;

	for(i = 0; i < size; i++){
		char oct = package[i];
		if(oct == F_FLAG || oct == ESC) {
			memmove(package + i + 2, package + i+1, size - i);
			if (oct == F_FLAG) {
				package[i+1] = XOR_7E_20;
				package[i] = ESC ;
			}
			else package[i+1] = XOR_7D_20;
		}
	}
	return size;
}

int deStuffing( char * package, int length){
	int size = length;

	int i;
	for(i = 0; i < size; i++){
		char oct = package[i];
		if(oct == ESC)
		{
			if(package[i+1] == XOR_7E_20){
				package[i] = F_FLAG;
				memmove(package + i + 1, package + i + 2,length - i + 2);

			}
			else if(package[i+1] == XOR_7D_20){
				memmove(package + i + 1, package + i + 2,length - i + 2);
			}
			size--;
		}
	}

	return size;
}

char makeBCC2(char* message, int length){
	char bcc = 0;
	int i;
	for(i = 0; i < length; i++){
		bcc ^= message[i];
	}
	return bcc;
}

int receiveSupervision(int fd,char * C)
{
	int status = 0;
	char A;
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
			status = receiveC(fd, C);
			break;
			case C_RCV:
			status = checkBCC(fd, A, *C);
			break;
			case BCC_RCV:
			status = receiveFlag(fd);
			if(status == FLAG_RCV)
			status = COMPLETE;
			//printf("full package!\n");}
			break;
		}

	}while(status != COMPLETE && flag == 0);
	//printf("Flag %d \n",flag);
	//printf("status %d \n",status);
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
	//printf("receiveC %x \n", (unsigned char)*ch);
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
	char* DISC = malloc(5*sizeof(char));
	DISC[0] = F_FLAG;
	DISC[1] = A_EM;
	DISC[2] = C_DISC;
	DISC[3] = A_EM ^ C_DISC;
	DISC[4] = F_FLAG;

	return DISC;
}

char* createRR(int package)
{
	char* RR = malloc(5*sizeof(char));
	RR[0] = F_FLAG;
	RR[1] = A_EM;
	if(package == 0)
	RR[2] = C_RR0;
	else if(package == 1)
	RR[2] = C_RR1;
	RR[3] = RR[1] ^ RR[2];
	RR[4] = F_FLAG;
	return RR;
}

char* createREJ(int package)
{
	char* REJ = malloc(5*sizeof(char));
	REJ[0] = F_FLAG;
	REJ[1] = A_EM;
	if(package == 0)
	REJ[2] = C_REJ0;
	else if(package == 1)
	REJ[2] = C_REJ1;
	REJ[3] = REJ[1] ^ REJ[2];
	REJ[4] = F_FLAG;
	return REJ;
}


int createAndSendPackage(int fd,int type)
{
	char * msg;

	switch(type){
		case SET_PACK:
		msg = createSet();
		break;
		case UA_PACK:
		msg = createUA();
		break;
		case DISC_PACK:
		msg = createDisc();
		break;
		case RR_0PACK:
		msg = createRR(0);
		break;
		case RR_1PACK:
		msg = createRR(1);
		break;
		case REJ_0PACK:
		msg = createREJ(0);
		break;
		case REJ_1PACK:
		msg = createREJ(1);
		break;
	}

	int res = sendMensage(fd,msg,5);

	free(msg);
	return res;
}

int createStart(char *filename, int length, unsigned int size, int type,char *package){
	package = realloc(package, length+10);
	package [0] = type;
	package [1] = 0;
	package [2] = 0x04;

	int i = 0;
	while(size != 0){
		package[3 + i] = size % 255;
		size = size % 255;
		i++;
	}

	package [3+i] = 1;
	package [4+i] = length;

	int j;
	int l = 5+i;
	for ( j = 0; j < length; j++){
		int l = l+j;
		package[l] = filename[j];
	}
	package[l+length] = makeBCC2(package,length + l);


	return (length + l + 1);
}

int packagePayLoad(int C, int size, char * payload){
	int tramaSize = size + 5;
	char * buffer = malloc(tramaSize);
	buffer[0] = F_FLAG;
	buffer[1] = A_EM;
	buffer[2] = C;
	buffer[3] = buffer[1] ^ buffer[2];
	int i;
	for(i = 0; i < size; i++)
	buffer[i + 4] = payload[i];

	buffer[tramaSize-1] = F_FLAG;
	memcpy(payload, buffer, tramaSize);

	free(buffer);
	/*
	printf("TRAMA I size:%d\n",tramaSize);
	for(i = 0; i < size +4 ; i++){
	printf("t%d:%x\n",i,payload[i]);
}*/

return tramaSize;
}

int sendMensage(int fd, char *message, int length)
{
	int res  = 0;
	while(res <= 0){
		res=write(fd, message, length);
	/*	int i;
		for(i = 0; i < length  ; i++)
		printf("t%d:%x\n",i,message[i]);*/
	}
	return res;
}

int checkRR_Reject(int C, unsigned char ch){
	if((C_RR1 == ch && C == 0) || (C_RR0 == ch && C==1))
	return COMPLETE;
	else
	return -1;
}

int extractPackage(char *package, char *trama,int length){
	// 5 is from F A C1 BBC1 |--| F
	int packageSize = length - 5;


	/*int i;
	for(i = 0; i < packageSize; i++)
		printf("i: %d - %x\n", i, package[i]);*/


	return packageSize;
}

int getTrama(int fd, char* trama){
	int size = 0;

	int flags = 0;
	int multi = 1;

	trama = realloc(trama, multi * TRAMA_SIZE);

	char ch;
	int res;
	printf("getting trama\n");

	while(flags <1){
		res = read(fd, &ch, 1);
		if(ch == F_FLAG && res > 0){
			flags++;
			size++;
			trama[size - 1] = ch;
		}
	}
	while(flags < 2){
		res = read(fd,&ch,1);
		if( res > 0){
	//		printf("package cell -- ");
			if(ch == F_FLAG){
				if(size == 1)
					continue;
				flags++;
			}


			size++;
			if(size > multi*TRAMA_SIZE)
			{
				multi *= 2;
				trama = realloc(trama, multi * TRAMA_SIZE);
			}
			trama[size - 1 ] = ch;
		}

	}

	//trama = realloc(trama, size);

	//VALIDATE
	printf("\ntrama received\n");
	if((trama[1] ^ trama[2]) == trama[3]){
		printf("BCC1 CHECK: TRUE\n" );
		return size;
	}
	else{
		printf("BCC1 CHECK: FALSE\n" );
		return -1;
	}

}

int openPort(char * port) {
	dataINFO.port = port;

	dataINFO.fd = open(dataINFO.port, O_RDWR | O_NOCTTY|O_NONBLOCK);
	if (dataINFO.fd <0) {
		perror(port);
		return 1;
	}

	if ( tcgetattr(dataINFO.fd,&dataINFO.oldtio) == -1) { // save current port settings
		perror("tcgetattr");
		return 1;
	}

	bzero(&dataINFO.newtio, sizeof(dataINFO.newtio));
	dataINFO.newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	dataINFO.newtio.c_iflag = IGNPAR;
	dataINFO.newtio.c_oflag = OPOST;
	// set input mode (non-canonical, no echo,...)
	dataINFO.newtio.c_lflag = 0;
	dataINFO.newtio.c_cc[VTIME]    = 0;   // inter-character timer unused
	dataINFO.newtio.c_cc[VMIN]     = 1;   // blocking read until 1 char received


	tcflush(dataINFO.fd, TCIFLUSH);

	if (tcsetattr(dataINFO.fd,TCSANOW,&dataINFO.newtio) == -1) {
		perror("tcsetattr");
		return 1;
	}

	return 0;
}

int closePort(){
	sleep(2);
	if ( tcsetattr(dataINFO.fd,TCSANOW,&dataINFO.oldtio) == -1) {
		perror("tcsetattr");
		return 1;
	}
	close(dataINFO.fd);
	return 0;
}
