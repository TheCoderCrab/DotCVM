#ifndef DEVICES_H
#define DEVICES_H

#include   <log.h>
#include   <file_utils.h>
#include   <fstream>
#include   <app_main.h>
#include   <optional>
#include   <maths.h>
#include   <stdio.h>
#include   <utils.h>
#include   <stdint.h>
#include   <dctypes.h>

#define BLOCK_SIZE      512
#define DISK_FILE       "disk.dat"
#define MEM_SIZE        50 * 1024 * 1024
#define DISK_SIZE       (100 * 1024 * 1024) / BLOCK_SIZE

#define SCR_BUFFER      0xA0000
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
    virtual ~IODevice() {}
    virtual void ioUpdate(byte ioLineData) = 0;
};

class Memory : public Device
{
private:
    dword m_Size;
    data* m_Data;
public:
    Memory(uint32_t size)
    {
        m_Size = size;
        m_Data = new data[m_Size];
        for(uint32_t i = 0; i < m_Size; i++)
            m_Data[i] = 0;
    }
    virtual ~Memory()
    {
        delete [] m_Data;
    }
    template<typename T> T& at(address adr)
    {
        return (T&)(m_Data[adr]);
    }
    byte& operator[](address adr)
    {
        return at<byte>(adr);
    }
    dword size() { return m_Size; }
    void update() override {}
    void exit() override {}
};

class Drive : public IODevice
{
private:
    FILE* m_DiskFile;
public:
    Drive(dword sizeIfNotPresentInBlocks)
    {
        fs::createIfNotExist(DISK_FILE, sizeIfNotPresentInBlocks * 512);
        m_DiskFile = fopen(DISK_FILE, "rb+");
        if(m_DiskFile == nullptr)
        {
            log("unable to open disk file!");
            debug("disk file: " << DISK_FILE);
            requestClose();
        }
        debug("Opened disk file successfully");
    }
    ~Drive()
    {
        fclose(m_DiskFile);
    }
    void ioUpdate(byte ioLineData) override
    {
        debug(ioLineData);
    }
    void update() override {}
    void exit() override {}
    void loadBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress)
    {
        debug("Reading block at: " << blockAddress << ", count: " << blockCount << ", to address: " << memAddress);
        if(fseek(m_DiskFile, blockAddress * 512, SEEK_SET) != 0)
        {
            log("Problem while trying to access disk\nMake sure you are not trying to read after the disk file");
            debug("While trying to read byte at " << blockAddress * 512);
            requestClose();
        }
        fread(&((*mem)[memAddress]), 512, blockCount, m_DiskFile);
        if(feof(m_DiskFile))
        {
            log("Reached end of disk file, can't read any further!");
            requestClose();
        }
        if(ferror(m_DiskFile))
        {
            log("Error while reading disk file!");
            requestClose();
        }
    }
};


class Screen : public IODevice
{
private:
    dword* m_ScrBuffer;
public:
    Screen(Memory* mem)
    {
        m_ScrBuffer = &(mem->at<dword>(SCR_BUFFER));
    }
    std::optional<Vector2> hasChanged()
    {
        for(uint32_t x = 0; x < SCR_WIDTH; x++)
            for(uint32_t y = 0; y < SCR_HEIGHT; y++)
                if(m_ScrBuffer[x + y * SCR_WIDTH] != (uint32_t) getPixel(x, y))
                    return Vector2{(int32_t) x, (int32_t) y};
        return {};
    }
    void setPixelAt(dword x, dword y, dword color)
    {
        m_ScrBuffer[x + y * SCR_WIDTH] = color;
    }
    virtual void ioUpdate(byte ioLineData) override
    {
        debug(ioLineData);
    }
    virtual void update() override
    {
        std::optional<Vector2> updateData = hasChanged();
        if(updateData)
        {

            debug("refreshing screen");
            for(uint32_t x = updateData->x; x < SCR_WIDTH; x++)
                for(int32_t y = updateData->y; y < SCR_HEIGHT; y++)
                    setPixel(x, y, (int32_t) m_ScrBuffer[x + y * SCR_WIDTH]);
            refreshScr();
        }

    }
    virtual void exit() override
    {
    }
};

// This is to prevent include loops and IDE complaining about classes not defined
#define CPU_INCLUDE_ERRORS_PROTECTION // Remove this line and you'll find hell waiting for you. Look at cpu.h for more info
#include <cpu.h> // This sould stay after all the devices definitions. Really.

namespace devices
{
    extern CPU* cpu;
    void init(dword memSize, dword diskSizeIfNotPresentInBlocks);
    void update();
    void exit();
}


#endif
