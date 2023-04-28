#include <stdio.h>
#include "../include/FileManagerLibrary.h"

int main() {
    ReadMBR("../Test/usb.img");
   // dumpMBR();
    readPartitions();
    return 0;
}
