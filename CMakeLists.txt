cmake_minimum_required(VERSION 3.25)
project(FileSystemManager C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES src/FileManagerLibrary.c include/FileManagerLibrary.h src/utility.c include/utility.h)
set(SOURCE_FILES_DATA CDataStructures/Src/LinkedList/LinkedList.c CDataStructures/Util/Utility.c CDataStructures/Src/Queue/Queue.c CDataStructures/Src/Stack/Stack.c CDataStructures/Test/TestRun.c CDataStructures/Test/LinkedListTest.c CDataStructures/Test/StackTest.c CDataStructures/Test/StackTest.c CDataStructures/Include/Stack.h CDataStructures/Include/LinkedList.h CDataStructures/Include/Queue.h CDataStructures/Include/Utility.h CDataStructures/Include/TestRun.h)

add_library(DataStructures ${SOURCE_FILES_DATA})

add_library(FileManagerLib ${SOURCE_FILES})

add_executable(FileSystemManager src/main.c)
target_link_libraries(FileManagerLib PRIVATE DataStructures)
target_link_libraries(FileSystemManager PRIVATE FileManagerLib)

