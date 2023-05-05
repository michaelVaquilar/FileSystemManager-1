#ifndef FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H
#define FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H

#include <stdint.h>

/**
 * Defines the structure of a partition in the MBR.
 */
typedef struct _partition {
    uint8_t bootable;
    uint8_t first_chs[3];
    uint8_t type;
    uint8_t last_chs[3];
    uint32_t lba_offset;
    uint32_t sector_count;
} __attribute__((packed)) partition;

/**
 * Defines the structure of a boot sector.
 */
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

/**
 * Defines the structure of the MBR.
 */
typedef struct _MBR {
    uint8_t bootcode[446];
    partition part[4];
    uint16_t signature;
} __attribute__((packed)) MBR;

/**
 * Defines the structure of a FAT16 table.
 */
typedef struct FAT16Table {
    uint16_t entry[32768];
} __attribute__((packed)) FAT16Table;

/**
 * Defines the structure of a root directory entry.
 */
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

typedef struct RootDirectory {
    RootDirectoryEntry entries[512];
    int count;
} ROOTDIRECTORY;

/**
 * Reads the Master Boot Record (MBR) from the specified file.
 *
 * @param filename the name of the file to read the MBR from.
 * @return 0 if the MBR was successfully read, otherwise returns a negative integer.
 */
int ReadMBR(const char* filename);

/**
 * Parses the USB device specified by the filename and populates the necessary data structures.
 *
 * @param filename the name of the USB device to parse.
 * @return 0 if the USB device was successfully parsed, otherwise returns a negative integer.
 */
int ParseUSB(const char* filename);

/**
 * Prints the contents of the MBR to the console.
 */
void dumpMBR();

/**
 * Prints the contents of the boot sector to the console.
 */
void dumpBootSector();

/**
 * Prints the parsed data from the USB device to the console.
 */
void printData();

/**
 * Reads the partitions in the MBR
 */
void readPartitions();

/**
 * Reads the LBA recieved from the Partitions.
 * @param offset location of the LBA
 * @return EXIT_SUCCESS OR EXIT_FAILURE
 */
int readLBA(uint32_t offset);

/**
 * Parses the fat tables via offset recieved from the LBA
 * @param offset location of the Fat Tables
 * @return EXIT_SUCCESS OR EXIT_FAILURE
 */
int readFatTables(uint32_t offset);

/**
 * Parses a directory given the offset of the directory
 * @param offset location of the Directory
 * @return EXIT_SUCCESS OR EXIT_FAILURE
 */
int ReadDir(uint32_t offset);

/**
 * Changes the current working directory to the specified directory.
 *
 * @param dirName the name of the directory to change to.
 * @return 0 if the directory was successfully changed, otherwise returns a negative integer.
 */
int ChangeDirectory(const char* dirName);

/**
 * Changes the current working directory back to the root directory.
 */
void backToRootDir();

/**
 * Prints the contents of the current working directory to the console.
 */
void dumpDir();

/**
 * Reads and prints the contents of the specified file to the console.
 *
 * @param filename the name of the file to read and print.
 */
void readFile(char *filename);

/**
 * Prints the contents of the File Allocation Table (FAT) to the console.
 */
void dumpFAT16Table();

#endif

