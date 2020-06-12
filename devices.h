#ifndef DEVICES_H
#define DEVICES_H

#include <log.h>
#include <file_utils.h>
#include <fstream>
#include <app_main.h>

#define DISK_FILE   "disk.dat"
#define MEM_SIZE    50 * 1024 * 1024
#define DISK_SIZE   (100 * 1024 * 1024) / 512

#define SCR_BUFFER  0xA0000

class Device
{
public:
    virtual void update() = 0;
    virtual void exit() = 0;
};

class IODevice : public Device
{
public:
    virtual void ioUpdate(unsigned int ioLineData) = 0;
};

template<typename T> class DataDevice : public Device
{
protected:
    unsigned int m_Size;
    T* m_Data;
public:
    ~DataDevice()
    {
        delete [] m_Data;
    }
    T* at(unsigned int address)
    {
        return &m_Data[address];
    }
};

class Memory : public DataDevice<unsigned char>
{
public:
    Memory(unsigned int size)
    {
        m_Size = size;
        m_Data = new unsigned char[m_Size];
        for(unsigned int i = 0; i < m_Size; i++)
            m_Data[i] = 0;
    }
    virtual void update() override {}
    virtual void exit() override {}
};

struct ByteBlock
{
    unsigned char data[512];
};

class Drive : public IODevice, public DataDevice<ByteBlock>
{
public:
    Drive(unsigned int sizeIfNotPresentInBlocks)
    {
        if(fs::exists(DISK_FILE))
        {
            log("Disk file found");
            log("Reading disk file");
            debug("Setting size");
            m_Size = fs::fileSize(DISK_FILE);
            debug("Getting disk File data");
            fs::FileData diskFileData = fs::readFile(DISK_FILE);
            debug("Disk file size " << diskFileData.size);

            if(diskFileData.size % 512 == 0)
            {
                debug("Disk file already setup correctly");
                m_Size = diskFileData.size / 512;
            }
            else
            {
                debug("Disk file should be aligned");
                m_Size = (unsigned int)(diskFileData.size / 512) + 1;
            }

            m_Data = new ByteBlock[m_Size];
            debug("Setting up data");
            for(unsigned int i = 0; i < m_Size; i++)
                for(unsigned int j = 0; j < 512; j++)
                    m_Data[i].data[j] = diskFileData.dataPtr[j + i * 512];
            log("Finished reading disk file");
        }
        else
        {
            log("Disk file not found");
            m_Size = sizeIfNotPresentInBlocks;
            m_Data = new ByteBlock[m_Size];
            for(unsigned int i = 0; i < m_Size; i++)
                for(unsigned int j = 0; j < 512; j++)
                    m_Data[i].data[j] = 0;
        }
    }
    virtual void ioUpdate(unsigned int ioLineData) override
    {
        debug(ioLineData);
    }
    virtual void update() override {}
    virtual void exit() override
    {
        log("Saving disk file");
        fs::deleteFile(DISK_FILE);
        std::ofstream diskFile(DISK_FILE, std::ios::out | std::ios::binary);
        for(unsigned int i = 0; i < m_Size; i++)
            for(unsigned int j = 0; j < 512; j++)
                diskFile << m_Data[i].data[j];
        log("Finished saving disk file");
    }
};


class Screen : public IODevice
{
private:
    unsigned int* m_ScrBuffer;
public:
    Screen(Memory* mem)
    {
        m_ScrBuffer = (unsigned int*) mem->at(SCR_BUFFER);
    }
    virtual void ioUpdate(unsigned int ioLineData) override
    {
        debug(ioLineData);
    }
    virtual void update() override
    {
        for(unsigned int x = 0; x < 720; x++)
            for(int y = 0; y < 480; y++)
                setPixel(x, y, m_ScrBuffer[x + y * 720]);
        refreshScr();
    }
    virtual void exit() override
    {

    }
};

namespace devices
{
    extern Memory* mem;
    extern Drive* drive;
    extern Screen* scr;

    void init(unsigned int memSize, unsigned int diskSizeIfNotPresentInBlocks);
    void update();
    void exit();
}


#endif
