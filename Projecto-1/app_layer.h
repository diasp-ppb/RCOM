#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"
#include "datalink.c"

/**
 Open File
 Return 0 if sucess
 Return -1 if fail
*/
int openFile(FILE ** file ,char * filename, char * mode);
/**
 Return file size
 Return -1 if size <= 0
*/
unsigned long getFileSize(FILE * file);

int transmitter(char * filename);
