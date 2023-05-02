
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
static ROOTDIRECTORY rootDir;
static FILE *fp;

const char* HumanNumberString(off_t size) {
    const off_t GIGABYTE = 1073741824;
    const off_t MEGABYTE = 1048576;
    const off_t KILOBYTE = 1024;

    static char hsize[32] = {0};
    memset(hsize, 0, sizeof(hsize));

    if (size >= GIGABYTE) {
        snprintf(hsize, sizeof(hsize), "%.2f GB", (double)size / GIGABYTE);
    } else if (size >= MEGABYTE) {
        snprintf(hsize, sizeof(hsize), "%.2f MB", (double)size / MEGABYTE);
    } else if (size >= KILOBYTE) {
        snprintf(hsize, sizeof(hsize), "%.2f KB", (double)size / KILOBYTE);
    } else {
        snprintf(hsize, sizeof(hsize), "%ld bytes", size);
    }

    return hsize;
}



void printData(){
    char label[12];
    memset(label, 0, sizeof(label));
    memcpy(label, bootSector.volumeName, sizeof(bootSector.volumeName));

    off_t size = (off_t)bootSector.numSectorsLarge * (off_t)bootSector.bytesPerSector;
    printf("-=| File System Information |=-\n");
    printf("--------------------------------------------------\n");
    printf("| # Label          Serial         Size     Type  |\n");
    printf("| 1  %s   %08X   %s    FAT16 |\n",label, bootSector.serialNumber, HumanNumberString(size));
    printf("--------------------------------------------------\n");
}














int ParseUSB(const char* filename){
    ReadMBR(filename);
    readPartitions();
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



    readRootDir(offset);

    return EXIT_SUCCESS;
}


int readRootDir(uint32_t offset) {
    memset(&rootDir, 0, sizeof(ROOTDIRECTORY));
    printf("Now reading root directory...\n");

    // Calculate the offset to the root directory
    uint32_t rootDirOffset = offset + bootSector.reservedSectors * bootSector.bytesPerSector + bootSector.sectorsPerFAT * bootSector.numCopiesOfFAT * bootSector.bytesPerSector;

    // Read the root directory entries
    int i;
    for (i = 0; i < bootSector.maxRootDirEntries; i++) {
        fseek(fp, rootDirOffset + i * sizeof(RootDirectoryEntry), SEEK_SET);
        fread(&rootDir.entries[i], sizeof(RootDirectoryEntry), 1, fp);

        // Check if the entry is empty
        if (rootDir.entries[i].filename[0] == 0x00) {
            break;
        }

        // Check if the entry is a long filename entry (not interested in these for now)
        if ((rootDir.entries[i].attributes & 0x0F) == 0x0F) {
            continue;
        }

        rootDir.count++;
    }

    return EXIT_SUCCESS;
}

void dumpRootDir() {
    printf("Dumping root directory:\n");
    int i;
    for (i = 0; i < rootDir.count; i++) {
        printf("Entry %d:\n", i);
        printf("  Filename: %s.%s\n", rootDir.entries[i].filename, rootDir.entries[i].ext);
        printf("  Attributes: 0x%02X\n", rootDir.entries[i].attributes);
        printf("  Starting cluster: %d\n", rootDir.entries[i].startingCluster);
        printf("  File size: %d bytes\n", rootDir.entries[i].fileSize);
    }
}


void dumpMBR(){
    printf("Boot code:\n");
    for (int i = 0; i < 512; i++) {
        printf("%02X ", mbr->bootcode[i]);
    }
}


