#include <stdio.h>
#include "../include/FileManagerLibrary.h"

int main() {
    ReadMBR("../Test/usb.img");
    //dumpMBR();
    //printPartitions();
    readPartitions();
    return 0;
}
