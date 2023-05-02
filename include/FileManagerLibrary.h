
#ifndef FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H
#define FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H

#include <stdint.h>

typedef struct _partition{
    uint8_t bootable;
    uint8_t first_chs[3];
    uint8_t type;
    uint8_t last_chs[3];
    uint32_t lba_offset;
    uint32_t sector_count;} __attribute__((packed)) partition;


typedef struct BootSector {
    uint8_t jumpCode[3];
    uint8_t oemName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numCopiesOfFAT;
    uint16_t maxRootDirEntries;
    uint16_t numSectorsSmall;
    uint8_t mediaDescriptor;
    uint16_t sectorsPerFAT;
    uint16_t sectorsPerTrack;
    uint16_t numHeads;
    uint32_t numHiddenSectors;
    uint32_t numSectorsLarge;
    uint16_t logicalDriveNumber;
    uint8_t extendedSignature;
    uint32_t serialNumber;
    uint8_t volumeName[11];
    uint8_t fatName[8];
    uint8_t executableCode[448];
    uint16_t executableMarker;
} __attribute__((packed)) BootSector;




typedef struct _MBR{
    uint8_t bootcode[446];
    partition part[4];
    uint16_t signature;} __attribute__((packed))MBR;


typedef struct FAT16Table {
    uint16_t entry[8192];   // each entry is 16 bits (2 bytes), and there are 8192 entries in a 16MB partition
} __attribute__((packed)) FAT16Table;



typedef struct RootDirectoryEntry {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint16_t reserved1;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t reserved2;
    uint16_t modifiedTime;
    uint16_t modifiedDate;
    uint16_t startingCluster;
    uint32_t fileSize;
} __attribute__((packed)) RootDirectoryEntry;


typedef struct RootDirectory{
    RootDirectoryEntry entries[512];
    int count;
} ROOTDIRECTORY;


int ReadMBR(const char* filename);

void ListContents(const char* filename);

void readPartitions();
uint32_t reverse_uint32(uint32_t num);
int readLBA(uint32_t offset);
void printPartitions();
int readRootDir(uint32_t offset);
void dumpRootDir();
int ParseUSB(const char* filename);
void dumpMBR();
void printData();

#endif