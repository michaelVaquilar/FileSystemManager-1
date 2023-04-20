cmake_minimum_required(VERSION 3.25)
project(FileSystemManager C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES src/FileManagerLibrary.c include/FileManagerLibrary.h)

add_library(FileManagerLib ${SOURCE_FILES})

add_executable(FileSystemManager src/main.c)

target_link_libraries(FileSystemManager PRIVATE FileManagerLib)
