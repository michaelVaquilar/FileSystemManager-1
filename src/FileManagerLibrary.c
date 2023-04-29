
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

    fseek(fp, offset, SEEK_SET);
    fread(&bootSector, sizeof(BootSector), 1, fp);

    if(bootSector.executableMarker != 0xAA55){
        printf("Boot Sector signature is not 0x55AA");
        return EXIT_FAILURE;
    }

    printf("Boot sector:\n");
    printf("Jump code + NOP: %02x %02x %02x\n", bootSector.jumpCode[0], bootSector.jumpCode[1], bootSector.jumpCode[2]);
    printf("OEM name: %s\n", bootSector.oemName);
    printf("Bytes per sector: %d\n", bootSector.bytesPerSector);
    printf("Sectors per cluster: %d\n", bootSector.sectorsPerCluster);
    printf("Reserved sectors: %d\n", bootSector.reservedSectors);
    printf("Number of copies of FAT: %d\n", bootSector.numCopiesOfFAT);
    printf("Maximum root directory entries: %d\n", bootSector.maxRootDirEntries);
    printf("Number of sectors in partition smaller than 32MB: %d\n", bootSector.numSectorsSmall);
    printf("Media descriptor: %02x\n", bootSector.mediaDescriptor);
    printf("Sectors per FAT: %d\n", bootSector.sectorsPerFAT);
    printf("Sectors per track: %d\n", bootSector.sectorsPerTrack);
    printf("Number of heads: %d\n", bootSector.numHeads);
    printf("Number of hidden sectors in partition: %u\n", bootSector.numHiddenSectors);
    printf("Number of sectors in partition: %u\n", bootSector.numSectorsLarge);
    printf("Logical drive number of partition: %d\n", bootSector.logicalDriveNumber);
    printf("Extended signature: %02x\n", bootSector.extendedSignature);
    printf("Serial number of partition: %u\n", bootSector.serialNumber);
    printf("Volume name of partition: %s\n", bootSector.volumeName);
    printf("FAT name: %s\n", bootSector.fatName);
    printf("Executable marker: %02x %02x\n", *((uint8_t*)&bootSector + 510), *((uint8_t*)&bootSector + 511));

    return EXIT_SUCCESS;
}




void dumpMBR(){
    printf("Boot code:\n");
    for (int i = 0; i < 512; i++) {
        printf("%02X ", mbr->bootcode[i]);
    }
}


//__________Directory Masaya____________

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

