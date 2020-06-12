#include <file_utils.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <stdio.h>

namespace fs
{
    bool exists(const char* filename)
    {
        struct stat buffer;
        return (stat(filename, &buffer) == 0);
    }

    void deleteFile(const char* fileName)
    {
        remove(fileName);
    }

    void createFile(const char* name, unsigned int size)
    {
        if(!exists(name))
        {
            std::ofstream diskFileCreate(name, std::ios::out | std::ios::binary);
            diskFileCreate.seekp(size - 1);
            diskFileCreate << (char) 0;
            diskFileCreate.close();
        }
        else
            std::cout << "Can't create an already existing file! " << name << std::endl;
    }

    void createIfNotExist(const char* name, unsigned int size)
    {
        if(!exists(name))
            createFile(name, size);
    }

    unsigned int fileSize(const char* filename)
    {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }
    FileData readFile(const char* name)
    {
        FileData data;
        data.size = fileSize(name);
        std::ifstream file(name, std::ios::in | std::ifstream::binary);
        data.dataPtr = new unsigned char[data.size];
        file.read((char*) data.dataPtr, data.size);
        return data;
    }
}
