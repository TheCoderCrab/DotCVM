#include <file_utils.h>
#include <log.h>
#include <stdio.h>
#include <unistd.h>



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
            FILE* file = fopen("myfile", "w");
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

    unsigned int fileSize(const char* filename)
    {
        FILE* file = fopen(filename, "rb");
        fseek(file, 0, SEEK_END);
        unsigned int size = ftell(file);
        fclose(file);
        return size;
    }
    FileData readFile(const char* name)
    {
        FileData data;
        data.size = fileSize(name);
        FILE* file = fopen(name, "rb");
        data.dataPtr = new uint8_t[data.size];
        fread(data.dataPtr, 1, data.size, file);
        fclose(file);
        return data;
    }
}
