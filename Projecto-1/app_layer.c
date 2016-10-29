#include "app_layer.h"

#define RECEIVER_MODE "w"
#define TRANSMITTER_MODE "r"

int mode ;

int main(int argc, char** argv){
    int mode = 3;
    if( argc == 3 && (strcmp("RECEIVER",argv[2]) == 0))
        mode = RECEIVER;
    else if( argc == 4  && (strcmp("TRANSMITTER",argv[2]) == 0))
        mode = TRANSMITTER;
    else {
        printf("Wrong number of arguments \n");
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0  RECEIVER\n");
        printf("      \tex: nserial /dev/ttyS0  TRANSMITTER  FilePath \n");
        exit(1);
    }

    if (  (strcmp("/dev/ttyS0", argv[1])!=0) &&  (strcmp("/dev/ttyS1", argv[1])!=0) ) {
        printf("Wrong seraial port: choose /dev/ttyS0 or /dev/ttyS1 \n");
        exit(1);
    }



//    llopen(argv[1], mode);

    if(mode == TRANSMITTER)
        transmitter(argv[3]);
    //else if (mode == RECEIVER);


//    llclose(mode);

    return 0;
}

int transmitter(char * filename){
    //TRANSMITTER
    printf("%s\n", filename);
    //OPEN FILE
    FILE *file = NULL ;

    if(openFile(&file, filename, "r") != 0)
    return 1;

    unsigned long size = getFileSize(file);

/*    int fd = fileno(file); // TODO MUDAR PARA PORTA FD
  //OPEN CONECTION
    if(0 != llopen(TRANSMITTER,fd))
    {
        printf("llopen fail: program will close .... \n");
        return 1;
    }*/

    int packSize;
    //START signal
    unsigned char *start = malloc(1);
    packSize = createStartEndPackage(START_PACK, filename, size, start);
//    llwrite(start, packSize, 0);
    free(start);

    //SEND File


    //END signal
    unsigned char *end = malloc(1);
    packSize = createStartEndPackage(END_PACK, filename, size, end);
//    llwrite(start, packSize, 0);
    free(end);

    //CLOSE CONECTION


    if(file != NULL)
    fclose(file);
    else{
        printf("File NULL\n");
    }

    return 0;
}

int receiver(){
    //RECEIVER
    //receive OPEN CONECTION request
    //receive START signal
    //receive and save  File
    //receive END signal
    //receive CLOSE CONECTION

    return 0;
}

/*
type = TRANSMITTER / RECEIVER

*/
int openFile(FILE ** file ,char * filename, char * mode){
    *file = fopen(filename, mode);
    if( *file == NULL){
        printf("File doens't exist! \n");
        return -1;
    }
    if(*file != NULL)
    printf("FILE EXIST\n" );
    return 0;
}

unsigned long getFileSize(FILE * file){
    unsigned long size = 0;
    int fd = fileno(file);

    lseek(fd, 0L, SEEK_END);
    size = ftell(file);
    lseek(fd, 0L, SEEK_SET);

    if(size <= 0){
        printf("File size is invalid, size: %lu\n", size);
        return -1;
    }

    printf("File size : %lu \n", size);
    return size;
}

int createStartEndPackage(int type, char* filename, int size, unsigned char* package)
{
    int nameLength = strlen(filename);
    int sizeLength = calculateNumBytes(size);

    int packSize = 5 + nameLength + sizeLength;
    package = realloc(package, packSize);

    package[0] = type;
    package[1] = TSIZE;
    package[2] = (char) sizeLength;

    int i = 3;
    while(size != 0){
        package[i] = (unsigned char) size;
        i++;
        size >>=8;
    } //size is written backwards

    package[i] = TNAME;
    ++i;
    package[i] = (char) nameLength;
    ++i;

    int j;
    for(j = 0; j < nameLength; i++, j++)
    package[i] = filename[j];

    /*    for(i = 0; i < packSize; i++)
    printf("%d - %x \n", i, package[i]);*/


    return packSize;
}

int createDataPackage(char *buffer, int size)
{
    int length = size + 4;
    buffer = realloc(buffer, length);

    memcpy(buffer + 4, buffer, size);
    buffer[0] = DATA_PACK;
    buffer[1] = 0; //TODO - what is N?
    buffer[2] = (char) (size / 256);
    buffer[3] = size % 256;

    int i;
    for(i = 0; i < length; i++)
    printf("%d - %x\n", i, buffer[i]);
    return length;
}


int calculateNumBytes(int num)
{
    int bytes = 0;
    while (num != 0) {
        num >>= 8;
        bytes++;
    }

    return bytes;
}
