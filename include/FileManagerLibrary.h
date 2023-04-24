
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


typedef struct _MBR{
    uint8_t bootcode[446];
    partition part[4];
    uint16_t magic;} __attribute__((packed))MBR;



typedef struct _DIR_ENTRY{
    uint8_t name8[8];
    uint8_t ext3[3];
    uint8_t attributes;
    uint16_t reserved;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
};


int ReadMBR(const char* filename);

void ListContents(const char* filename);

#endif //FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H
