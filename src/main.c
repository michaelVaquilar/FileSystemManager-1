#include <stdio.h>
#include "../include/FileManagerLibrary.h"

int main() {
    ReadMBR("../Test/usb.img");
    
    ListContents("../Test/usb.img");
    return 0;
}
