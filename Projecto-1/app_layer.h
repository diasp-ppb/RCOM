#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"
#include "datalink.c"

#define TSIZE 0
#define TNAME 1

#define PACKSIZE 5

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

int createDataPackage(char *buffer, int size);

int transmitter(char * filename);
int receiver();

int createStartEndPackage(int type, char* filename, int size, char* package);
int calculateNumBytes(int num);

int getFileInfo(char* buffer, int buffsize, int *size, char *name);
int getData(char *buffer, int size);
