
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
    uint16_t start_sector_cylinder; // combine start_sector and start_cylinder
    uint8_t type;
    uint8_t end_head;
    uint16_t end_sector_cylinder; // combine end_sector and end_cylinder
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

typedef struct _volume_id {
    uint8_t jmp[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved_nt;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) volume_id;


int ReadMBR(const char* filename);

void ListContents(const char* filename);

int GetNameFromEntry(DIR_ENTRY* entry, char* name);
void readPartitions();
uint32_t reverse_uint32(uint32_t num);

void readVolumeID(uint32_t lbaBegin);
void dumpMBR();
int readSector( uint32_t lba, uint8_t *buffer);

#endif //FILESYSTEMMANAGER_FILEMANAGERLIBRARY_H
