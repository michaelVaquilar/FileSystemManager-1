/*
 * UTILITY.C/.H
 *
 * This file is part of the CSC430 FAT File Format Project.
 *   https://github.com/prof-tallman)
 *   Copyright (c) 2021
 *   Concordia CSC430
 *    Joshua Tallman
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#include "../include/utility.h"



const char* PartitionTypeString(uint8_t type_code)
{
    /*
     * Creates a string for printing the partition type (FAT32, NTFS, etc).
     */

    const char* none = "None";
    const char* fat12 = "FAT12";
    const char* fat16 = "FAT16";
    const char* fat32 = "FAT32";
    const char* ntfs = "NTFS/HPFS";
    const char* other = "Other";

    switch(type_code)
    {
        case 0x00:
            return none;
        case 0x01:
            return fat12;
        case 0x06:
        case 0x0e:
            return fat16;
        case 0x0b:
        case 0x0c:
            return fat32;
        case 0x07:
            return ntfs;
        default:
            return other;
    }
}


const char* MediaTypeString(int media_type)
{
    /*
     * Creates a string for printing the FAT media type (HDD, RAM, Floppy).
     */

    const char* mtype_hdd = "HDD";
    const char* mtype_ram = "RAM Drive";
    const char* mtype_floppy = "Floppy";
    const char* mtype_other = "Other";

    switch(media_type)
    {
        case 0xf8:
            return mtype_hdd;
        case 0xfa:
            return mtype_ram;
        case 0xf0:
            return mtype_floppy;
        default:
            return mtype_other;
    }
}


const char* HumanNumberString(uint64_t size)
{
    /*
     * Creates a string for printing a 64-bit number using the human-readable
     * kilo/mega/giga suffixes. The function chooses the suffix based on the
     * magnitude of the number. Large numbers will have many digits because
     * the largest suffix is giga (no tera, peta, exo, etc).
     */

    const size_t GIGABYTE = 1073741824;
    const size_t MEGABYTE = 1048576;
    const size_t KILOBYTE = 1024;

    static char hsize[32] = {0};
    memset(hsize, 0, sizeof(hsize));
    if (size >= GIGABYTE)
    {
        sprintf(hsize, "%.2lf GB", (double)(size / GIGABYTE));
    }
    else if (size >= MEGABYTE)
    {
        sprintf(hsize, "%.2lf MB", (double)(size / MEGABYTE));
    }
    else if (size >= KILOBYTE)
    {
        sprintf(hsize, "%.2lf KB", (double)(size / KILOBYTE));
    }
    else
    {
        sprintf(hsize, "%lu bytes", size);
    }
    return hsize;
}


const char* FileAttributeString(uint8_t attribute)
{
    /*
     * Creates a six character string that encodes a file attribute descriptor.
     * Each attribute has a specific location in the string. Missing attributes
     * are noted with a dash sign. The six possible attributes are (A)rchive,
     * (D)irectory, (V)olume, (S)ystem, (H)idden, and (R)ead-only.
     */

    // Don't use the "------" literal string because literals are read-only
    static char description[7] = {0};
    strcpy(description, "------");

    if ((attribute & FILE_ATTRIBUTE_DIRECTORY) > 0) description[1] = 'D';
    if ((attribute & FILE_ATTRIBUTE_ARCHIVE) > 0) description[0] = 'A';

    // Special case for the Long File Name attribute
    if ((attribute & FILE_ATTRIBUTE_LFN) == FILE_ATTRIBUTE_LFN)
    {
        description[2] = 'L';
        description[3] = 'O';
        description[4] = 'N';
        description[5] = 'G';
    }
    else
    {
        if ((attribute & FILE_ATTRIBUTE_VOLUME) > 0) description[2] = 'V';
        if ((attribute & FILE_ATTRIBUTE_SYSTEM) > 0) description[3] = 'S';
        if ((attribute & FILE_ATTRIBUTE_HIDDEN) > 0) description[4] = 'H';
        if ((attribute & FILE_ATTRIBUTE_READONLY) > 0) description[5] = 'R';
    }

    return description;
}


static void RemoveTrailingSpaces(char* buffer)
{
    /*
     * Removes trailing spaces from the right side of a NULL terminated string.
     * Starts at the end of the string and works backwards, changing every
     * space character to a NULL character. Stops when it reaches the beginning
     * of the string or when it reaches a character that is not a space (0x20).
     *
     * Function assumes that the string is NULL terminated and writeable.
     */

    assert(buffer != NULL);
    int end = strlen(buffer);

    // Substitute NULLs in place of the trailing spaces
    int i = end - 1;
    while(i >= 0 && buffer[i] == ' ')
    {
        buffer[i] = '\0';
        i--;
    }
}


const char* EightDotThreeString(const uint8_t name[8], const uint8_t ext[3])
{
    /*
     * Creates a NULL terminated filename string by concatinating the file's
     * name and extension. The FAT file system stores the name and extension in
     * an 8 byte buffer and a 3 byte buffer, but neither are NULL terminated.
     */

    static char full_filename[13] = {0};
    memset(full_filename, 0, sizeof(full_filename));
    strncat(full_filename, (char*)name, 8);
    RemoveTrailingSpaces(full_filename);

    if (ext[0] != ' ')
    {
        strcat(full_filename, ".");
        strncat(full_filename, (char*)ext, 3);
        RemoveTrailingSpaces(full_filename);
    }
    return full_filename;
}


const char* GetPathSeparator(const char* path)
{
    /*
     * Returns the most likely separator character ('/' or '\\') for a given
     * file path. The function works by searching the path string for a forward
     * slash (for macOS/Linux) or a blackslash (for Windows).
     */

    // First, check for a forward-slash because it is the path separator on
    // macOS and it is not allowed as part of a Windows filename. So if there's
    // a forward slash present, it must be the path separater. In contrast,
    // backslashes *are* allowed in macOS filenames (even though they are rare)

    // Linux and macOS
    if (strchr(path, '/') != NULL)
    {
       return "/";
    }

    // Windows
    else if (strchr(path, '\\') != NULL)
    {
        return "\\";
    }

    // Default to Windows
    else
    {
        return "\\";
    }
}


static const char* CreateHexLine(const char* buffer, int start, int count)
{
    /*
     * Creates a single line for an 8-byte long hex dump, but does *not* print
     * anything. The string will include a maximum of 8 bytes of data. If you
     * want to dump more than 8 bytes, call this function in a loop. The output
     * string will begin with the hex data's hex representation and it will end
     * with the ASCII representation. If the data contains any non-printable
     * characters, they will be shown as a dot '.'.
     */

    static char hex[80] = {0};
    memset(hex, 0, sizeof(hex));
    if (start < count)
    {
        int pos = 0;

        // HEX DUMP
        // The first byte is special because it isn't prefixed with a space
        // The last 7 bytes can be printed in a loop
        pos += sprintf(&hex[pos], "%02x", buffer[start] & 0xFF);
        for(int i = start+1; i < start+8 && i < count; i++)
        {
            pos += sprintf(&hex[pos], " %02x", buffer[i] & 0xFF);
        }

        // ASCII DUMP
        // The first byte is special because it contains a column separator
        // (some extra spaces). The last 7 bytes can be printed in a loop
        if (buffer[start] >= 20 && buffer[start] < 127)
        {
            pos += sprintf(&hex[pos], "  %c", buffer[start]);
        }
        else
        {
            pos += sprintf(&hex[pos], "  .");
        }
        for(int i = start+1; i < start+8 && i < count; i++)
        {
            if (buffer[i] >= 20 && buffer[i] < 127)
            {
                pos += sprintf(&hex[pos], "%c", buffer[i]);
            }
            else
            {
                pos += sprintf(&hex[pos], ".");
            }
        }
    }
    return hex;
}


void PrintHexDump(const char* buffer, int count)
{
    /*
     * Prints an arbitrary buffer to stdout in a traditional hex-dump format.
     * Each line will display a maximum of 8 bytes.
     */
     
    for (int i = 0; i < count; i += 8)
    {
        printf("%s\n", CreateHexLine(buffer, i, count));
    }
}
