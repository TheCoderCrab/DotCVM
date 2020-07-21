#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <DotCVM/utils/types.h>

namespace fs
{
    struct FileData
    {
        data* dataPtr;
        uint32_t size;
        ~FileData()
        {
            delete []  dataPtr;
        }
    };
    void deleteFile(const char* fileName);
    bool exists(const char* filename);
    void createFile(const char* name, uint32_t size);
    void createIfNotExist(const char* name, uint32_t size);
    unsigned int fileSize(const char* filename);
    FileData readFile(const char* name);
}

#endif
