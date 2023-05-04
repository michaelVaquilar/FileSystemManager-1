
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
#define OPEN_SLOT 0xE5


static MBR *mbr;
static BootSector bootSector;
static ROOTDIRECTORY rootDir;
static FILE *fp;
static FAT16Table* fatTable;
static FileSystem* fs;


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

void addChild(Directory* parent, RootDirectoryEntry entry) {
    Directory* child = (Directory*)malloc(sizeof(Directory));
    child->entry = entry;
    child->parent = parent;
    child->children = NULL;
    child->childCount = 0;

    if (parent->children == NULL) {
        parent->children = (Directory*)malloc(sizeof(Directory) * 1);
    } else {
        Directory* temp = (Directory*)realloc(parent->children, sizeof(Directory) * (parent->childCount + 1)); //sigsegv happening here
        if (temp == NULL) {
            printf("Memory reallocation failed.\n");
            free(child);
            return;
        }
        parent->children = temp;
    }
    parent->children[parent->childCount] = *child;
    parent->childCount++;
    free(child);
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
    readFatTables(offset);
    return EXIT_SUCCESS;
}

int readFatTables(uint32_t offset) {
    // Determine the starting sector of the FAT table
    uint32_t fatStartSector = bootSector.reservedSectors + (bootSector.numCopiesOfFAT * bootSector.sectorsPerFAT);

    fatTable = malloc(sizeof(FAT16Table));
    if (fatTable == NULL) {
        perror("Error allocating memory for FAT table");
        fclose(fp);
        return -1;
    }

    // Seek to the starting sector of the FAT table
    if (fseek(fp, fatStartSector * bootSector.bytesPerSector, SEEK_SET) != 0) {
        perror("Error seeking to FAT table start sector");
        fclose(fp);
        return -1;
    }

    uint8_t fatTableData[bootSector.numCopiesOfFAT * bootSector.sectorsPerFAT * bootSector.bytesPerSector];
    if (fread(fatTableData, sizeof(fatTableData), 1, fp) != 1) {
        perror("Error reading FAT table data");
        fclose(fp);
        return -1;
    }

    memcpy(fatTable->entry, fatTableData, sizeof(fatTable->entry));
    readRootDir(offset);

    return 0;
}



int readRootDir(uint32_t offset) {
    fs = malloc(sizeof(FileSystem));

    if (!fs) {
        perror("Error allocating memory for FileSystem");
        fclose(fp);
        return -1;
    }
    fs->root = NULL;


    memset(&rootDir, 0, sizeof(ROOTDIRECTORY));
    printf("Now reading root directory...\n");

    // Calculate the offset to the root directory
    uint32_t rootDirOffset = offset + bootSector.reservedSectors * bootSector.bytesPerSector + bootSector.sectorsPerFAT * bootSector.numCopiesOfFAT * bootSector.bytesPerSector;

    fs->root = (Directory*)malloc(sizeof(Directory));
    fs->root->parent = NULL;
    fs->root->children = NULL;
    fs->root->childCount = 0;

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

        addChild(fs->root, rootDir.entries[i]);

        // Check if the entry is a subdirectory
        if (rootDir.entries[i].attributes & 0x10) {
            uint16_t startingCluster = rootDir.entries[i].startingCluster;
            readSubDir(offset, startingCluster, &fs->root->children[fs->root->childCount - 1]);
        }
    }

    return EXIT_SUCCESS;
}

int readSubDir(uint32_t offset, uint16_t startingCluster, Directory* currentDir) {
    uint32_t clusterOffset = (bootSector.reservedSectors + bootSector.numCopiesOfFAT * bootSector.sectorsPerFAT) * bootSector.bytesPerSector;
    uint32_t clusterSize = bootSector.sectorsPerCluster * bootSector.bytesPerSector;
    uint32_t subDirOffset = offset + clusterOffset + (startingCluster - 2) * clusterSize;

    int i = 0;
    while (1) {
        fseek(fp, subDirOffset + i * sizeof(RootDirectoryEntry), SEEK_SET);
        RootDirectoryEntry subDirEntry;
        fread(&subDirEntry, sizeof(RootDirectoryEntry), 1, fp);

        // Check if the entry is empty
        if (subDirEntry.filename[0] == 0x00) {
            break;
        }

        // Check if the entry is a long filename entry (not interested in these for now)
        if ((subDirEntry.attributes & 0x0F) == 0x0F) {
            i++;
            continue;
        }

        addChild(currentDir, subDirEntry);

        // Check if the entry is a subdirectory
        if (subDirEntry.attributes & 0x10) {
            readSubDir(offset, subDirEntry.startingCluster, &currentDir->children[currentDir->childCount - 1]);
        }

        i++;
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

void ListContents(const char* filename) {
    // Open the image disk file
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }

    // Calculate the location of the root directory
    int rootDirLocation = 0x141000;
    //Skip the boot sector and FAT sector
    fseek(fp, rootDirLocation, SEEK_SET);

    int entryFound = 0;
    // Read the root directory entries
    DIR_ENTRY entry;
    for (int i = 0; i < 224; i++) {
        fread(&entry, sizeof(DIR_ENTRY), 1, fp);
        // Check if the directory entry represents a file or subdirectory
        if (entry.name8[0] == 0x00 || entry.name8[0] == 0xE5) {
            continue;
        }
        else if (entry.attributes & DIRECTORY_ATTRIBUTE_SUBDIRECTORY) {
            printf("<DIR> ");
        }
        else if (entry.attributes & DIRECTORY_ATTRIBUTE_ARCHIVE) {
            printf("<FILE> ");
        }
        else {
            // Skip other entries
            continue;
        }

        // Extract the file or directory name
        char name[13];
        int len = GetNameFromEntry(&entry, name);
        uint32_t cluster = entry.first_cluster_hi << 16 | entry.first_cluster_lo;
        printf("%s %d\n", name, cluster);

    }

    // Close the disk image file
    fclose(fp);
}

void ReadFileFromCluster(const char* filename, int cluster) {
    // Open the image disk file
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }

    BPB bpb;
    // Calculate the location of the First Data Sector
    int FirstDataSector = bpb.BPB_RsvdSecCnt + bpb.BPB_NumFATs * bpb.BPB_FATSz32;
    int FirstSectorofCluster = ((cluster - 2) * bpb.BPB_SecPerClus) + FirstDataSector;

    // Seek to the first sector of the cluster
    fseek(fp, FirstSectorofCluster * bpb.BPB_BytePerSec, SEEK_SET);

    // Read the contents of the file
    char buffer[bpb.BPB_SecPerClus * bpb.BPB_BytePerSec];
    fread(buffer, 1, bpb.BPB_SecPerClus * bpb.BPB_BytePerSec, fp);

    // Print the contents of the file
    printf("%s", buffer);

    // Close the disk image file
    fclose(fp);
}



