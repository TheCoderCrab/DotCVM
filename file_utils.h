#ifndef FILE_UTILS_H
#define FILE_UTILS_H

namespace fs
{
    struct FileData
    {
        unsigned char* dataPtr;
        unsigned int size;
        ~FileData()
        {
            delete []  dataPtr;
        }
    };
    void deleteFile(const char* fileName);
    bool exists(const char* filename);
    void createFile(const char* name, unsigned int size);
    void createIfNotExist(const char* name, unsigned int size);
    unsigned int fileSize(const char* filename);
    FileData readFile(const char* name);
}

#endif
