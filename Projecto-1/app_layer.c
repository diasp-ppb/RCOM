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



    llopen(argv[1], mode);

    if(mode == TRANSMITTER)
        transmitter(argv[3]);
    else if (mode == RECEIVER)
        receiver();

    llclose(mode);

    return 0;
}

int transmitter(char * filename){
    //TRANSMITTER
    printf("%s\n", filename);
    //OPEN FILE
    FILE *file = NULL ;

    if(openFile(&file, filename, "r+") != 0)
    return 1;

    unsigned long size = getFileSize(file);
    int bytesWritten = 0;


    int packSize;
    //START signal
    char *start = malloc(1);
    packSize = createStartEndPackage(START_PACK, filename, size, start);
    while(llwrite(start, packSize, 0) != COMPLETE){}
    free(start);

    //SEND File
    char *data = malloc(PACKSIZE);
    char C = 1;
    while(bytesWritten < size)
    {
      int res = fread(data, 1, PACKSIZE, file);
      int bytesRead = res;

    //  int i;
  /*    for(i = 0; i < res ; i ++){
        printf("%d - %x\n",i, data[i]);
      }
*/
      res =  createDataPackage(data, res);


    /*  for(i = 0; i < res ; i ++){
        printf("%d - %x\n",i, data[i]);
      }*/
      while(llwrite(data, res, C) != COMPLETE){}
      bytesWritten += bytesRead;
      C ^= 1;
      printf("written: %d - total: %d\n", bytesRead, bytesWritten);
    }

    free(data);


    //END signal
    char *end = malloc(1);
    packSize = createStartEndPackage(END_PACK, filename, size, end);
    while(llwrite(end, packSize, 0) != COMPLETE){}
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

    //receive START signal
    char *start = malloc(1);
    int startSize = llread(start, 0);
    int size;
    char *name = malloc(1);
    getFileInfo(start, startSize, &size, name);
    printf("Start Read name: %s - size: %d \n", name, size);


    FILE *file = NULL;
    if(openFile(&file, name, "w+") != 0)
      printf("opened file\n");


  /*  if(createFile(file, name) != 0)
    {
      printf("error creating file\n");
      return 1;
    }*/
    free(name);


    //receive and save  File
    int bytesRead = 0;
    char C = 1;
    char *buffer = malloc(PACKSIZE);
    while(bytesRead < size){
      int size;
      size = llread(buffer, C); //TODO - change C
      size = getData(buffer, size);
      if(size > 0)
      {
        fwrite(buffer, 1, size, file);
        bytesRead += size;
        C ^= 1;
        printf("total read: %d\n", bytesRead);
      }
    }
    free(buffer);


    //receive END signal
    char *end = malloc(1);
    int endSize = llread(end, 0);
    name = malloc(1);
    getFileInfo(end, endSize, &size, name);
    printf("End Read name: %s - size: %d \n", name, size);
    free(name);


    return 0;
}

/*
type = TRANSMITTER / RECEIVER

*/
int openFile(FILE ** file ,char * filename, char * mode){
    *file = fopen(filename, mode);
    if( *file == NULL){
        printf("File doesn't exist! \n");
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

int createStartEndPackage(int type, char* filename, int size, char* package)
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

  //      for(i = 0; i < packSize; i++)
  //  printf("%d - %x \n", i, package[i]);
/*
    getFileInfo(package, packSize, NULL, NULL);*/

    return packSize;
}

int createDataPackage(char *buffer, int size)
{
    int length = size + 4;
    char * copy = malloc(size);
    memcpy(copy, buffer, size);
    buffer = realloc(buffer, length);

    buffer[0] = DATA_PACK;
    buffer[1] = 0; //TODO - what is N?
    buffer[2] = (unsigned char) (size / 256);
    buffer[3] = (unsigned char) (size % 256);

    memcpy(buffer+4, copy, size);
/*    int i;
    for(i = 0; i < length; i++)
        printf("%d - %x\n", i, buffer[i]);

    getData(buffer, length);*/
    free (copy);
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

int getFileInfo(char* buffer, int buffsize, int *size, char *name)
{
    int fsize = 0;
    int sizeLength = (int) buffer[2];
    if(buffer[1] == TSIZE)
    {
        printf("sizeLength :%d\n", sizeLength);
        int i;
        for(i = 0; i < sizeLength; i++)
        {
            int j;
            unsigned char ch = (unsigned char) buffer[3+i];
            unsigned int curr = (unsigned int) ch;
            for(j = 0; j < i; j++)
                curr = curr << 8;
            fsize += curr;
        }
        *size = fsize;
      //  printf("size: %d\n", fsize);
    }
    else
        return 1;

    int i = 3 + sizeLength;
    if(buffer[i] == TNAME)
    {
        ++i;
        int nameLength = (int) buffer[i];
        ++i;
        printf("nameLength: %d\n", nameLength);
        name = realloc(name, nameLength);
        int j;
        for(j = 0; j < nameLength; j++)
          name[j] = buffer[i + j];
        name[j]='\0';

    //    printf("name: %s\n", name);
    }
    else
        return 1;
    return 0;
}

int getData(char *buffer, int size)
{
    int length = buffer[2] * 256 + buffer[3];

    char *copy = malloc(length);

    memcpy(copy, buffer + 4, length);
    memcpy(buffer, copy, length);

    free(copy);

    return length;
}
