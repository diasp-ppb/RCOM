

#include "app_layer.h"

#define RECEIVER_MODE "w"
#define TRANSMITTER_MODE "r"

int mode ;

int main(int argc, char** argv){

  if ( argc < 3 || argc > 4) {
          printf("Wrong number of arguments \n");
          printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0  RECEIVER\n");
          printf("      \tex: nserial /dev/ttyS0  TRANSMITTER  FilePath \n");
    exit(1);
  }

  if( argc == 3 &&  strcmp("RECEIVER",argv[2]) != 0)
  {
    // mode = RECEIVER;
  }
  else if( argc = 4  && strcmp("TRANSMITTER",argv[2]) != 0){
  // mode = TRANS;
  }

return 0;
}

int transmitter(){
    //TRANSMITTER

    //OPEN FILE
    
    //OPEN CONECTION
	
    //START signal
    //SEND File
    //END signal
    //CLOSE CONECTION

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
int openFile(FILE * file ,char * filename, char * mode){ 
 file = fopen(filename, mode);
if( file == NULL){
 	printf("File doens't exist! \n");
	return -1;
}
return 0;
}
int getFileSize(FILE * file){
  int size = 0;
  
	lseek(file, 0L, SEEK_END);
	size = ftell(file);
	lseek(file, 0L, SEEK_SET);  
 	 if(size <= 0){
 		 printf("File size is invalid, size: %d\n", size);
 		 return -1;
 	}	
 prinf("File size : %d \n", size);
 return size;
}


