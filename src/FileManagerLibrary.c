
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


void ListContents(const char* filename) {
    // Open the image disk file
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }

    // Calculate the location of the root directory
    int rootDirLocation = 512 + (1 + (2 * 32)) * 512; // FAT Beginning + reserved sector + #FATs * sectors/FAT * bytes/sector
    // Just check the location
    printf("Root directory location: %d\n", rootDirLocation);
    //Skip the boot sector FAT sector
    fseek(fp, rootDirLocation, SEEK_SET);

    // Read the root directory entries
    char buffer[32]; //Each entry is 32 bytes
    for (int i = 0; i < 224; i++) {
        fread(buffer, sizeof(buffer), 1, fp);
        // Check if the directory entry represents a file or subdirectory
        if (buffer[0] == 0x00) {
            continue;  // Unused or deleted entry
        }
        if (buffer[11] == 0x10) {
            printf("<DIR> ");
        }
        else {
            printf("<FILE> ");
        }
        
        // Extract the file or directory name
        char name[9];
        strncpy(name, buffer, 8);
        name[8] = '\0';
        char extension[4];
        strncpy(extension, buffer + 8, 3);
        extension[3] = '\0';
        printf("%s%s\n", name, extension);

    }
    // Close the disk image file
    fclose(fp);
}
