## FileSystemManager

The file system is an important and highly visible part of the operating system. It allows you to efficiently store, organize, and retrieve data. Many file systems access physical devices such as hard drives, solid-state drives, tape drives, or even RAM (these are called RAM drives and they lose all data when powered down). Others are virtual devices, meaning that the "disk drive" is not so much a physical device but rather it is a file stored on a larger, host file system. This is how disk images and virtual machines work.

This project parses an img file and reads the partitions within to find the Root Directory and navigate to other directories.

## Documentation
For detailed documentation, see [Documentation.pdf](https://github.com/tensign1444/FileSystemManager/blob/master/Documentation.pdf)

## Goals

Our team accomplished many of our goals for this project, although we faced significant challenges in understanding the layout of FAT12, FAT16, and FAT32. Despite our best efforts, we were unable to implement one idea we had; this was to use our [data structures library stack](https://github.com/tensign1444/CDataStructures) to track the user's last directory visited. Instead, we could only send the user back to the root directory. While this was a disappointment, we are proud of the progress we made and the valuable experience we gained from working on such a difficult project.

## Usage
To use the FileSystemManager library, simply clone this repository into your project directory and include the filemanagerlibrary.h header file in your code. You can then call the library functions to read and manipulate FAT16 file systems.

## Contributing
If you would like to contribute to the FileSystemManager library, please fork this repository and submit a pull request with your changes. All contributions are welcome!

## Authors
[@Masa-dotcom](https://github.com/Masa-dotcom), [@michaelVaquilar](https://github.com/michaelVaquilar), [@tensign1444](https://github.com/tensign1444)

Thank you to Professor Tallman for allowing us the use his Utility file.

## License
FileSystemManager is released under the MIT License. See LICENSE for more information.
