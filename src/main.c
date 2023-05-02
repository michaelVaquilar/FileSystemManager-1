#include <stdio.h>
#include "../include/FileManagerLibrary.h"
#include <stdio.h>
#include "../include/FileManagerLibrary.h"
#include <stdio.h>
#include <string.h>

void print_help() {
    printf("Commands:\n");
    printf("  help         Display this help message\n");
    printf("  hello        Greet the user with a slight intro\n");
    printf("  ls           List files\n");
    printf("  CAT          Reads each file parameter in sequence\n"
           "               and writes it to standard output\n");
    printf("  EXPORT       Marks an environment variable to be exported\n"
           "               with any newly forked child processes\n");
    printf("  CD           Move between directories\n");
    printf("  PWD          Writes the full path of the directory\n");
    printf("  CP           Copies files from one location to another\n");
    printf("  MV           Moves files to another location\n");
    printf("  IMPORT       Inserts data from an external file\n");
}

int main() {
    ParseUSB("../Test/usb.img");
    char command[100];
    printData();
    while (1) {

        printf("C:/ > ");
        fgets(command, 100, stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "hello") == 0) {
            printf("Hello, I am a virtual file system. I will try my best to complete given tasks!\n");
        } else if (strcmp(command, "ls") == 0) {
            printf("Output ls\n");
        } else if (strcmp(command, "CAT") == 0) {
            printf("Output CAT\n");

        } else if (strcmp(command, "EXPORT") == 0) {
            printf("Output EXPORT\n");

        } else if (strcmp(command, "CD") == 0) {
            printf("Output CD\n");

        } else if (strcmp(command, "CP") == 0) {
            printf("Output CP\n");

        } else if (strcmp(command, "MV") == 0) {
            printf("Output MV\n");

        } else if (strcmp(command, "IMPORT") == 0) {
            printf("Output IMPORT\n");

        } else if (strcmp(command, "PWD") == 0) {
            printf("Output PWD\n");
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            printf("Unknown command...\n"
                   "Please try again\n");
        }
    }
}