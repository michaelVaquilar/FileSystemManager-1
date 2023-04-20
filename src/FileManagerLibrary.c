
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/FileManagerLibrary.h"

static char g_working_directory[256] = {0};
static FILE* g_file_pointer = NULL;
static int g_partition_count = 0;
MBR *mbr;

const char* HumanNumberString(off_t size){
    const off_t GIGABYTE = 1073741824;
    const off_t MEGABYTE = 1048576;
    const off_t KILOBYTE = 1024;

    static char hsize[32] = {0};
    memset(hsize,0,sizeof(hsize));
}


int ReadMBR(const char* filename) {
   struct stat fs = {0};
   int rc = stat(filename, &fs);

   FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error opening disk image file.\n");
        return EXIT_FAILURE;
    }
   rc = fseek(fp,0, SEEK_SET);

   char* buffer = (char*)malloc(sizeof(MBR));

   memset(buffer,0,sizeof(MBR));
   int count = fread(buffer,sizeof(char), sizeof(MBR), fp);

   mbr = (MBR*)buffer;
}


void dumpMBR(){
    for (int i = 0; i < 446; i++) {
        printf("%02X ", mbr->bootcode[i]);
    }
}

