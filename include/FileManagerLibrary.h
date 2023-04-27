
#ifndef FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H
#define FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H

#include <stdint.h>

/**typedef struct _partition{
    uint8_t bootable;
    uint8_t first_chs[3];
    uint8_t type;
    uint8_t last_chs[3];
    uint32_t lba_offset;
    uint32_t sector_count;} __attribute__((packed)) partition;**/


typedef struct _partition {
    uint8_t status;
    uint8_t start_head;
    uint16_t start_sector : 6;
    uint16_t start_cylinder : 10;
    uint8_t type;
    uint8_t end_head;
    uint16_t end_sector : 6;
    uint16_t end_cylinder : 10;
    uint32_t start_lba;
    uint32_t size;
} __attribute__((packed)) partition;


typedef struct _MBR{
    uint8_t bootcode[446];
    partition part[4];
    uint16_t signature;} __attribute__((packed))MBR;



#define SECTOR_SIZE 512

typedef struct _DIR_ENTRY {
    uint8_t name8[8];
    uint8_t ext3[3];
    uint8_t attributes;
    uint16_t reserved;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_hi;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t first_cluster_lo;
    uint32_t file_size;
} __attribute__((packed)) DIR_ENTRY;


int ReadMBR(const char* filename);

void ListContents(const char* filename);

int GetNameFromEntry(DIR_ENTRY* entry, char* name);
void readPartitions();
void dumpMBR();
void readVolumeID(uint32_t lbaBegin);

#endif //FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H
