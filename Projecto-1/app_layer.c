
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int mode ;
int main(int argc, char** argv){

  if ( (argc < 3 || argc > 4) {
          printf("Wrong number of arguments \n");
          printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0  RECEIVER\n");
          printf("      \tex: nserial /dev/ttyS0  TRANSMITTER  FilePath \n");
    exit(1);
  }

  if( argc == 3 &&  strcmp("RECEIVER",argv[2]) != 0)
  {
    // mode = TRANSMITTER;
  }
  else if( argc = 4  && strcmp("TRANSMITTER",argv[2]) != 0){
  // mode = RECEIVER;
  }





    //TRANSMITTER
    //OPEN CONECTION
    //START signal
    //SEND File
    //END signal
    //CLOSE CONECTION


    //RECEIVER
    //receive OPEN CONECTION request
    //receive START signal
    //receive and save  File
    //receive END signal
    //receive CLOSE CONECTION


return 0;
}


int getFileSize(){
  return 0;
}
