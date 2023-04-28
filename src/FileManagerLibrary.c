
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/FileManagerLibrary.h"

#define DIRECTORY_ATTRIBUTE_SUBDIRECTORY 0x10
#define MAGICNUMBER 0x55AA


static FILE *fp;
static MBR *mbr;

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
    mbr = (MBR*)buffer;

    mbr->signature = (uint16_t) (buffer[sizeof(MBR)-1] << 8 | buffer[sizeof(MBR)-2]);
    if (mbr->signature != 0xAA55) {
        printf("Invalid MBR signature.\n");
        free(buffer);
        fclose(fp);
        return EXIT_FAILURE;
    }

    free(buffer);

    return EXIT_SUCCESS;
}

uint32_t reverse_uint32(uint32_t num) {
    return ((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000);
}


void readPartitions(const char *filename) {
    int i;
    for (i = 0; i < 4; i++) {
        partition p = mbr->part[i];
        const char *fs_type = NULL;
        if (p.type == 0x01 || p.type == 0x04 || p.type == 0x06 || p.type == 0x0E) {
            fs_type = "FAT12/FAT16";
        } else if (p.type == 0x0B || p.type == 0x0C) {
            fs_type = "FAT32";
        }

        if (fs_type) {
            uint32_t lbaBegin = reverse_uint32(p.start_lba);
            uint32_t fileOffset = lbaBegin * 512; // Calculate file offset
            printf("\n %s partition at LBA 0x%08X (file offset: 0x%08X) in partition %d\n", fs_type, lbaBegin, fileOffset, i + 1);
            uint32_t numSectors = reverse_uint32(p.size);
            uint32_t sectorSize = 512;
            uint32_t filesystemSize = numSectors * sectorSize;
            printf("Partition size: %u bytes (%u sectors)\n", filesystemSize, numSectors);

            readVolumeID(lbaBegin);
            // do something with lbaBegin and filesystemSize, e.g. read the filesystem data from the disk
        }
    }
}




void readVolumeID(uint32_t lbaBegin) {
    uint8_t sectorBuffer[512];
    int result = readSector(lbaBegin, sectorBuffer);
    if (result != EXIT_SUCCESS) {
        printf("Error reading Volume ID sector.\n");
        return;
    }

    printf("Sector contents:\n");
    for (int i = 0; i < 512; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }
        printf("%02X ", sectorBuffer[i]);
    }
    printf("\n");

    // FAT32: Volume label is at offset 0x47 (71) in the boot sector
    char volume_label[12];
    memcpy(volume_label, &sectorBuffer[71], 11);
    volume_label[11] = '\0';

    volume_id *vID = (volume_id *)sectorBuffer;
    printf("Volume ID: %08X\n", vID->volume_id);
    printf("Volume Label: %s\n", volume_label);
}


int readSector( uint32_t lba, uint8_t *buffer) {
    if (fp == NULL) {
        printf("Error opening disk image file.\n");
        return EXIT_FAILURE;
    }
    uint32_t sectorSize = 512;
    uint32_t offset = lba * sectorSize;
    int rc = fseek(fp, offset, SEEK_SET);
    if (rc != 0) {
        printf("Error seeking to LBA %u.\n", lba);
        fclose(fp);
        return EXIT_FAILURE;
    }

    size_t count = fread(buffer, sizeof(uint8_t), sectorSize, fp);
    if (count != sectorSize) {
        printf("Error reading sector at LBA %u.\n", lba);
        fclose(fp);
        return EXIT_FAILURE;
    }

    fclose(fp);
    return EXIT_SUCCESS;
}



void dumpMBR(){
    printf("Boot code:\n");
    for (int i = 0 + 16; i< 512; i++) {
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
