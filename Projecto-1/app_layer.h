#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/**
 Open File 
 Return 0 if sucess
 Return -1 if fail
*/
int openFile(FILE * file ,char * filename, char * mode);
/**
 Return file size
 Return -1 if size <= 0
*/
int getFileSize(FILE * file);
