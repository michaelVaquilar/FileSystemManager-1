#pragma once

#include <stdint.h>


#define FILE_ATTRIBUTE_READONLY 0x01
#define FILE_ATTRIBUTE_HIDDEN 0x02
#define FILE_ATTRIBUTE_SYSTEM 0x04
#define FILE_ATTRIBUTE_VOLUME 0x08
#define FILE_ATTRIBUTE_LFN \
    (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | \
     FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_VOLUME)
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_ARCHIVE 0x20

const char* PartitionTypeString(uint8_t type_code);
const char* FileAttributeString(uint8_t attribute);
const char* MediaTypeString(int media_type);
const char* HumanNumberString(uint64_t size);
const char* EightDotThreeString(const uint8_t name[8], const uint8_t ext[3]);
const char* GetPathSeparator(const char* path);

void PrintHexDump(const char* buffer, int count);
