
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/FileManagerLibrary.h"
#include "../include/utility.h"
#include "../CDataStructures/Include/Stack.h"
#include "../CDataStructures/Include/Utility.h"

#define DIRECTORY_ATTRIBUTE_SUBDIRECTORY 0x10

static MBR *mbr;
static BootSector bootSector;
static ROOTDIRECTORY rootDir;
static FILE *fp;
static FAT16Table* fatTable;
static int currentPartition;
static STACK *offSetStack;
static uint32_t currentPartitionOffset;
static uint32_t rootOffset;

void printData(){
    char label[12];
    memset(label, 0, sizeof(label));
    memcpy(label, bootSector.volumeName, sizeof(bootSector.volumeName));

    off_t size = (off_t)bootSector.numSectorsLarge * (off_t)bootSector.bytesPerSector;
    printf("         -=| File System Information |=-          \n");
    printf("--------------------------------------------------\n");
    printf("| # Label          Serial         Size     Type  |\n");
    printf("| %d %s   %08X  %s   %.8s |\n",currentPartition, label, bootSector.serialNumber, HumanNumberString(size), bootSector.fatName);
    printf("--------------------------------------------------\n");
}

int ParseUSB(const char* filename){
    offSetStack = InitStack((compare) compare_int32_t);
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
    currentPartition = 0;
    int i;
    for (i = 0; i < 4; i++) {
        partition p = mbr->part[i];
        if (p.type == 0x06 || p.type == 0x0E) {
            currentPartition++;
            currentPartitionOffset = p.lba_offset * 512;
            readLBA(currentPartitionOffset);
            break;
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
    readFatTables(offset);
    return EXIT_SUCCESS;
}

int readFatTables(uint32_t offset) {
    uint32_t fatStartSector = bootSector.reservedSectors;
    int fatTableOffset = offset + fatStartSector * 512;
    fatTable = malloc(sizeof(FAT16Table));
    if (fatTable == NULL) {
        perror("Error allocating memory for FAT table");
        fclose(fp);
        return -1;
    }
    if (fseek(fp, fatTableOffset, SEEK_SET) != 0) {
        perror("Error seeking to FAT table start sector");
        fclose(fp);
        return -1;
    }
    uint32_t fatTableSize = bootSector.sectorsPerFAT * bootSector.bytesPerSector;
    uint16_t fatTableData[fatTableSize / 2];
    if (fread(fatTableData, sizeof(uint16_t), fatTableSize / 2, fp) != fatTableSize / 2) {
        perror("Error reading FAT table data");
        fclose(fp);
        return -1;
    }
    memcpy(fatTable->entry, fatTableData, sizeof(fatTable->entry));
    rootOffset = fatTableOffset + 512 * 2 * bootSector.sectorsPerFAT;
    ReadDir(rootOffset);
    return 0;
}

int ReadDir(uint32_t offset) {
    Push(offSetStack, &offset);
    ROOTDIRECTORY tempRootDir;
    memset(&tempRootDir, 0, sizeof(ROOTDIRECTORY));

    int i;
    for (i = 0; i < bootSector.maxRootDirEntries; i++) {
        fseek(fp, offset + i * sizeof(RootDirectoryEntry), SEEK_SET);
        fread(&tempRootDir.entries[i], sizeof(RootDirectoryEntry), 1, fp);
        if (tempRootDir.entries[i].filename[0] == 0x00) {
            break;
        }
        if ((tempRootDir.entries[i].attributes & 0x0F) == 0x0F) {
            tempRootDir.count++;
            continue;
        }
        if (tempRootDir.entries[i].attributes == 0x0F) {
            printf("long");
        }
        tempRootDir.count++;
    }
    memcpy(&rootDir, &tempRootDir, sizeof(ROOTDIRECTORY));
    return EXIT_SUCCESS;
}

uint32_t getOffset(char *subDirectoryName) {
    RootDirectoryEntry *subDirEntry = NULL;
    for (int i = 0; i < rootDir.count; i++) {
        char *entryName = EightDotThreeString(rootDir.entries[i].filename, rootDir.entries[i].ext);
        if (strcasecmp(entryName, subDirectoryName) == 0 && (rootDir.entries[i].attributes & 0x10)) {
            subDirEntry = &rootDir.entries[i];
            break;
        }
    }
    if (subDirEntry == NULL) {
        printf("Subdirectory not found\n");
        return 0;
    }
    uint16_t subDirCluster = subDirEntry->startingCluster;
    uint32_t dataAreaOffset = (bootSector.reservedSectors + (bootSector.numCopiesOfFAT * bootSector.sectorsPerFAT) + bootSector.maxRootDirEntries * 32 / bootSector.bytesPerSector) * bootSector.bytesPerSector;
    uint32_t clusterOffset = (subDirCluster - 2) * bootSector.sectorsPerCluster * bootSector.bytesPerSector;
    return currentPartitionOffset + dataAreaOffset + clusterOffset;
}

void changeDirectory(char *subDirectoryName) {
    uint32_t offset = getOffset(subDirectoryName);
    ReadDir(offset);
}

void backToRootDir(){
    int i;
    for(i = 0; i < offSetStack->Count; i++){
        Pop(offSetStack);
    }
    ReadDir(rootOffset);
}

void dumpDir() {
    int i;
    for (i = 0; i < rootDir.count; i++) {
        printf("%s ", EightDotThreeString(rootDir.entries[i].filename, rootDir.entries[i].ext));
    }
    printf("%s\n", EightDotThreeString(rootDir.entries[i + 1].filename, rootDir.entries[i + 1].ext));
}

void dumpMBR(){
    printf("Boot code:\n");
    for (int i = 0; i < 512; i++) {
        printf("%02X ", mbr->bootcode[i]);
    }
}

void dumpBootSector() {
    printf("Boot Sector Contents:\n");
    printf("Jump Code: %02x %02x %02x\n", bootSector.jumpCode[0], bootSector.jumpCode[1], bootSector.jumpCode[2]);
    printf("OEM Name: %.8s\n", bootSector.oemName);
    printf("Bytes Per Sector: %u\n", bootSector.bytesPerSector);
    printf("Sectors Per Cluster: %u\n", bootSector.sectorsPerCluster);
    printf("Reserved Sectors: %u\n", bootSector.reservedSectors);
    printf("Number of Copies of FAT: %u\n", bootSector.numCopiesOfFAT);
    printf("Max Root Directory Entries: %u\n", bootSector.maxRootDirEntries);
    printf("Number of Sectors (Small): %u\n", bootSector.numSectorsSmall);
    printf("Media Descriptor: %02x\n", bootSector.mediaDescriptor);
    printf("Sectors Per FAT: %u\n", bootSector.sectorsPerFAT);
    printf("Sectors Per Track: %u\n", bootSector.sectorsPerTrack);
    printf("Number of Heads: %u\n", bootSector.numHeads);
    printf("Number of Hidden Sectors: %u\n", bootSector.numHiddenSectors);
    printf("Number of Sectors (Large): %u\n", bootSector.numSectorsLarge);
    printf("Logical Drive Number: %u\n", bootSector.logicalDriveNumber);
    printf("Extended Signature: %02x\n", bootSector.extendedSignature);
    printf("Serial Number: %08x\n", bootSector.serialNumber);
    printf("Volume Name: %.11s\n", bootSector.volumeName);
    printf("FAT Name: %.8s\n", bootSector.fatName);
    printf("Executable Marker: %04x\n", bootSector.executableMarker);
}

void dumpFAT16Table() {
    if (fatTable == NULL) {
        printf("FAT16 table is not initialized.\n");
        return;
    }
    printf("FAT16 Table Entries:\n");
    for (int i = 0; i < 5; i++) {
            printf("Entry %d: %04X\n", i, fatTable->entry[i]);
    }
}

void readFile(char *filename) {
    RootDirectoryEntry *fileEntry = NULL;
    for (int i = 0; i < rootDir.count; i++) {
        char *entryName = EightDotThreeString(rootDir.entries[i].filename, rootDir.entries[i].ext);
        if (strcasecmp(entryName, filename) == 0 && !(rootDir.entries[i].attributes & DIRECTORY_ATTRIBUTE_SUBDIRECTORY)) {
            fileEntry = &rootDir.entries[i];
            break;
        }
    }
    if (fileEntry == NULL) {
        printf("File not found\n");
        return;
    }
    uint16_t fileCluster = fileEntry->startingCluster;
    uint32_t dataAreaOffset = (bootSector.reservedSectors + (bootSector.numCopiesOfFAT * bootSector.sectorsPerFAT) + bootSector.maxRootDirEntries * 32 / bootSector.bytesPerSector) * bootSector.bytesPerSector;
    uint32_t clusterOffset = (fileCluster - 2) * bootSector.sectorsPerCluster * bootSector.bytesPerSector;
    uint32_t fileOffset = currentPartitionOffset + dataAreaOffset + clusterOffset;

    fseek(fp, fileOffset, SEEK_SET);
    char *fileData = malloc(fileEntry->fileSize);

    fread(fileData, 1, fileEntry->fileSize, fp);
    printf("%s\n", fileData);
    free(fileData);
}

