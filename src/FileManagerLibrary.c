
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/FileManagerLibrary.h"

#define DIRECTORY_ATTRIBUTE_SUBDIRECTORY 0x10
#define MAGICNUMBER 0x55AA


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

    rc = fseek(fp, 0, SEEK_SET);
    char* buffer = (char*)malloc(sizeof(MBR));
    memset(buffer, 0, sizeof(MBR));
    int count = fread(buffer, sizeof(char), sizeof(MBR), fp);
    mbr = (MBR*)buffer;

    mbr->signature = (uint16_t) (buffer[sizeof(MBR)-1] << 8 | buffer[sizeof(MBR)-2]);
    if (mbr->signature != 0xAA55) {
        printf("Invalid MBR signature.\n");
        free(buffer);
        fclose(fp);
        return EXIT_FAILURE;
    }

    fclose(fp);
    free(buffer);

    return EXIT_SUCCESS;
}


void readPartitions() {
    int i;
    for (i = 0; i < 4; i++) {
        partition p = mbr->part[i];
        if (p.type == 0x0B || p.type == 0x0C) {
            uint32_t lbaBegin = p.start_lba;
            printf("\n FAT32 partition at LBA %u in partition %d\n", lbaBegin, i + 1);
            uint32_t partitionSize = p.size;
            uint32_t sectorSize = 512;
            uint32_t filesystemSize = partitionSize * sectorSize;
            printf("Partition size: %u bytes\n", partitionSize);
            printf("Filesystem size: %u bytes\n", filesystemSize);
            readVolumeID(lbaBegin);
            // do something with lbaBegin and filesystemSize, e.g. read the filesystem data from the disk
        }
    }
}


void readVolumeID(uint32_t lbaBegin) {
    // Read the first sector of the FAT32 filesystem (the Volume ID)
    uint32_t sectorSize = 512;
    uint8_t* sectorBuffer = (uint8_t*)malloc(sectorSize);
    readSector(lbaBegin, 1, sectorBuffer);

    // Extract information from the Volume ID sector
    uint8_t sectorsPerCluster = sectorBuffer[0x0D];
    uint16_t reservedSectorCount = *((uint16_t*)&sectorBuffer[0x0E]);
    uint8_t fatCount = sectorBuffer[0x10];
    uint32_t sectorsPerFat = *((uint32_t*)&sectorBuffer[0x24]);
    char volumeID[12];
    memcpy(volumeID, &sectorBuffer[0x2B], 11);
    volumeID[11] = '\0';

    // Print the extracted information to the console
    printf("Volume ID: %s\n", volumeID);
    printf("Sectors per cluster: %u\n", sectorsPerCluster);
    printf("Reserved sector count: %u\n", reservedSectorCount);
    printf("FAT count: %u\n", fatCount);
    printf("Sectors per FAT: %u\n", sectorsPerFat);

    // Free the sector buffer
    free(sectorBuffer);
}



void dumpMBR(){
    printf("Boot code:\n");
    for (int i = 0; i < 512; i++) {
        printf("%02X ", mbr->bootcode[i]);
    }
}

int GetNameFromEntry(DIR_ENTRY* entry, char* name) {
    int i;
    for (i = 0; i < 8; i++) {
        if (entry->name8[i] == ' ') {
            break;
        }
        name[i] = entry->name8[i];
    }
    int hasExt = 0;
    for (int j = 0; j < 3; j++) {
        if (entry->ext3[j] == ' ') {
            break;
        }
        if (!hasExt) {
            name[i] = '.';
            i++;
            hasExt = 1;
        }
        name[i] = entry->ext3[j];
        i++;
    }
    name[i] = '\0';
    return i;
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
    DIR_ENTRY entry;
    for (int i = 0; i < 224; i++) {
        fread(&entry, sizeof(DIR_ENTRY), 1, fp);
        // Check if the directory entry represents a file or subdirectory
        if (entry.name8[0] == 0x00) {
            continue;  // Unused or deleted entry
        }
        if (entry.attributes & DIRECTORY_ATTRIBUTE_SUBDIRECTORY) {
            printf("<DIR> ");
        }
        else {
            printf("<FILE> ");
        }

        // Extract the file or directory name
        char name[13];
        int len = GetNameFromEntry(&entry, name);
        printf("%s\n", name);
    }
    // Close the disk image file
    fclose(fp);
}
