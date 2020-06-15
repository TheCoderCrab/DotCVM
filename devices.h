#ifndef DEVICES_H
#define DEVICES_H

#include <log.h>
#include <file_utils.h>
#include <fstream>
#include <app_main.h>
#include <optional>
#include <maths.h>

#define DISK_FILE   "disk.dat"
#define MEM_SIZE    50 * 1024 * 1024
#define DISK_SIZE   (100 * 1024 * 1024) / 512

#define SCR_BUFFER  0xA0000
#define SCR_WIDTH       720
#define SCR_HEIGHT      480

class Device
{
public:
    virtual void update() = 0;
    virtual void exit() = 0;
};

class IODevice : public Device
{
public:
    virtual ~IODevice()
    {

    }
    virtual void ioUpdate(unsigned int ioLineData) = 0;
};

template<typename T> class DataDevice : public Device
{
protected:
    unsigned int m_Size;
    T* m_Data;
public:
    virtual ~DataDevice()
    {
        debug("Destructing DATA DEVICE");
        delete [] m_Data;
    }
    T& operator[](unsigned int address)
    {
        return m_Data[address];
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
    public:
    unsigned int* m_ScrBuffer;
public:
    Screen(Memory* mem)
    {
        m_ScrBuffer = (unsigned int*) &((*mem)[SCR_BUFFER]);
    }
    std::optional<Vector2> hasChanged()
    {
        for(unsigned int x = 0; x < SCR_WIDTH; x++)
            for(unsigned int y = 0; y < SCR_HEIGHT; y++)
                if(m_ScrBuffer[x + y * SCR_WIDTH] != (unsigned int) getPixel(x, y))
                    return Vector2{(int) x, (int) y};
        return {};
    }
    void setPixelAt(unsigned int x, unsigned int y, unsigned int color)
    {
        m_ScrBuffer[x + y * SCR_WIDTH] = color;
    }
    virtual void ioUpdate(unsigned int ioLineData) override
    {
        debug(ioLineData);
    }
    virtual void update() override
    {
        std::optional<Vector2> updateData = hasChanged();
        if(updateData)
        {

            debug("refreshing screen");
            for(unsigned int x = updateData->x; x < SCR_WIDTH; x++)
                for(int y = updateData->y; y < SCR_HEIGHT; y++)
                    setPixel(x, y, (int) m_ScrBuffer[x + y * SCR_WIDTH]);
            refreshScr();
        }

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
