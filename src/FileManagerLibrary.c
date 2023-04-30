
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/FileManagerLibrary.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/FileManagerLibrary.h"

#define DIRECTORY_ATTRIBUTE_SUBDIRECTORY 0x10
#define MAGICNUMBER 0x55AA


static MBR *mbr;
static BootSector bootSector;;
static FILE *fp;

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
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error opening disk image file.\n");
        return EXIT_FAILURE;
    }

    rc = fseek(fp, 0, SEEK_SET);
    char* buffer = (char*)malloc(sizeof(MBR));
    memset(buffer, 0, sizeof(MBR));
    int count = fread(buffer, sizeof(char), sizeof(MBR), fp);
    mbr = (MBR*) buffer;


    if (mbr->signature != 0xAA55) {
        printf("Invalid MBR signature.\n");
        free(buffer);
        fclose(fp);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

void readPartitions() {
    int i;
    for (i = 0; i < 4; i++) {
        partition p = mbr->part[i];
        if (p.type == 0x06 || p.type == 0x0E) {
            printf("\nLBA offset is %u in partition %d\n", p.lba_offset * 512, i + 1);
            readLBA(p.lba_offset * 512);
        }
    }
}


int readLBA(uint32_t offset) {
    // Read the boot sector
    fseek(fp, offset, SEEK_SET);
    fread(&bootSector, sizeof(BootSector), 1, fp);

    if(bootSector.executableMarker != 0xAA55){
        printf("Boot Sector signature is not 0x55AA");
        return EXIT_FAILURE;
    }

    // Calculate the offset to the root directory
    uint32_t rootDirOffset = offset + bootSector.reservedSectors * bootSector.bytesPerSector + bootSector.sectorsPerFAT * bootSector.numCopiesOfFAT * bootSector.bytesPerSector;

    // Read the root directory entries
    RootDirectoryEntry dirEntry;
    int i;
    for (i = 0; i < bootSector.maxRootDirEntries; i++) {
        fseek(fp, rootDirOffset + i * sizeof(RootDirectoryEntry), SEEK_SET);
        fread(&dirEntry, sizeof(RootDirectoryEntry), 1, fp);

        // Check if the entry is empty
        if (dirEntry.filename[0] == 0x00) {
            break;
        }

        // Check if the entry is a long filename entry (not interested in these for now)
        if ((dirEntry.attributes & 0x0F) == 0x0F) {
            continue;
        }

        // Print the entry's filename
        printf("%s.%s\n", dirEntry.filename, dirEntry.ext);
    }

    return EXIT_SUCCESS;
}





void dumpMBR(){
    printf("Boot code:\n");
    for (int i = 0; i < 512; i++) {
        printf("%02X ", mbr->bootcode[i]);
    }
}


