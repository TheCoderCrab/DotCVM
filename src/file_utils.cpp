#include <file_utils.h>
#include <log.h>
#include <stdio.h>
#include <unistd.h>
#include <app_main.h>
#include <dctypes.h>


namespace fs
{
    bool exists(const char* filename)
    {
        return access(filename, F_OK) != -1;
    }

    void deleteFile(const char* fileName)
    {
        remove(fileName);
    }

    void createFile(const char* name, uint32_t size)
    {
        if(!exists(name))
        {
            FILE* file = fopen(name, "wb");
            fseek(file, size, SEEK_SET);
            fputc('\0', file);
            fclose(file);
        }
        else
            debug("Can't create an already existing file! " << name);
    }

    void createIfNotExist(const char* name, uint32_t size)
    {
        if(!exists(name))
            createFile(name, size);
    }

    uint32_t fileSize(const char* filename)
    {
        if(!exists(filename))
        {
            debug("Can't request size of inexisting file!");
            requestClose();
        }
        FILE* file = fopen(filename, "r");
        fseek(file, 0, SEEK_END);
        uint32_t size = ftell(file);
        fclose(file);
        return size;
    }
    FileData readFile(const char* name)
    {
        FileData fileData;
        fileData.size = fileSize(name);
        FILE* file = fopen(name, "rb");
        fileData.dataPtr = new data[fileData.size];
        fread(fileData.dataPtr, 1, fileData.size, file);
        fclose(file);
        return fileData;
    }
}
