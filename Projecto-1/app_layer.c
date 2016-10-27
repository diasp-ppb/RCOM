#include "app_layer.h"

#define RECEIVER_MODE "w"
#define TRANSMITTER_MODE "r"

int mode ;

int main(int argc, char** argv){
/*
  if ( argc < 3 || argc > 4) {
          printf("Wrong number of arguments \n");
          printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0  RECEIVER\n");
          printf("      \tex: nserial /dev/ttyS0  TRANSMITTER  FilePath \n");
    exit(1);
  }

  if( argc == 3 && (strcmp("RECEIVER",argv[2]) != 0))
  {
    // mode = RECEIVER;
  }
  else if( argc == 4  && (strcmp("TRANSMITTER",argv[2]) != 0)){
  // mode = TRANS;
  }
*/






return 0;
}

int transmitter(char * filename){
    //TRANSMITTER

    //OPEN FILE

    FILE *file = NULL ;


    if(openFile(&file, filename, "r") != 0)
      return 1;

    unsigned long size = getFileSize(file);


    int fd = fileno(file); // TODO MUDAR PARA PORTA FD
    //OPEN CONECTION
    if(0 != llopen(TRANSMITTER,fd))
    {
      printf("Program will close .... \n");
      return 1;
    }
    //START signal
    //SEND File
    //END signal
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
