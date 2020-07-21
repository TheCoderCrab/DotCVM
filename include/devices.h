// TODO: make io invalid operations set iof flag instead of requestClose

#ifndef DEVICES_H
#define DEVICES_H

#include <log.h>
#include <file_utils.h>
#include <fstream>
#include <app_main.h>
#include <optional>
#include <stdio.h>
#include <dc_utils.h>
#include <stdint.h>
#include <dctypes.h>
#include <err_code.h>
#include <io.h>
#include <string.h>
#include <window.h>

#define BLOCK_SIZE      512
#define DISK_FILE       "disk.dat"
#define DISK_SIZE       (100 * 1024 * 1024) / BLOCK_SIZE

#define MEM_SIZE        50 * 1024 * 1024

#define SCR_BUFFER      0xA0000
#define SCR_WIDTH       720
#define SCR_HEIGHT      480

class Memory
{
private:
    data* m_Data;
    dword m_Size;
public:
    Memory(uint32_t size)
    {
        debug("Creating a new instance of Memory");
        m_Size = size;
        debug("Memory size set");
        m_Data = new data[m_Size];
        debug("Allocated space for machine memory");
        debug("Memory created");
    }
    ~Memory()
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
    dword out()
    {
        return m_Size;
    }
};

class Drive
{
private:
    FILE*       m_DiskFile;
    dword       m_Size;
    IOInputMode m_InputMode;
    dword       m_io_BlockCount;
    address     m_io_BlockAdr;
    address     m_io_MemAdr;
    dword       m_io_LastInstruction;

public:
    Drive(dword sizeIfNotPresentInBlocks)
    {
        debug("Creating drive");
        m_InputMode = IOInputMode::INSTRUCTION;
        fs::createIfNotExist(DISK_FILE, sizeIfNotPresentInBlocks * 512);
        m_DiskFile = fopen(DISK_FILE, "rb+");
        if(m_DiskFile == nullptr)
        {
            debug("disk file: " << DISK_FILE);
            requestClose(FILE_OPEN_FAILURE, "Unable to open disk file!");
        }
        m_Size = fs::fileSize(DISK_FILE) / 512;
        debug("Opened disk file successfully");
        debug("Drive created");
    }
    ~Drive()
    {
        fclose(m_DiskFile);
    }
    void loadBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress)
    {
        debug("Reading block at: " << blockAddress << ", count: " << blockCount << ", to address: " << memAddress);
        if(fseek(m_DiskFile, blockAddress * BLOCK_SIZE, SEEK_SET) != 0)
        {
            debug("error while trying to set cursor to byte at " << blockAddress * 512);
            requestClose(FILE_READ_FAILURE, "Problem while trying to access disk\nMake sure you are not trying to read after the disk file");
        }
        fread(&((*mem)[memAddress]), BLOCK_SIZE, blockCount, m_DiskFile);
        if(feof(m_DiskFile))
            requestClose(FILE_READ_FAILURE, "Reached end of disk file, can't read any further!");
        if(ferror(m_DiskFile))
            requestClose(FILE_ERROR, "Error while reading disk file!");
    }
    void storeBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress)
    {
        debug("Writing block from address: " << memAddress << ", count: " << blockCount << ", to drive at: " << blockAddress);
        if(fseek(m_DiskFile, blockAddress * BLOCK_SIZE, SEEK_SET) != 0)
        {
            debug("error while trying to set cursor to byte at " << blockAddress * 512);
            requestClose(FILE_READ_FAILURE, "Problem while trying to access disk\nMake sure you are not trying to read after the disk file");
        }
        fwrite(&((*mem)[memAddress]), BLOCK_SIZE, blockCount, m_DiskFile);
        if(feof(m_DiskFile))
            requestClose(FILE_READ_FAILURE, "Reached end of disk file, can't read any further!");
        if(ferror(m_DiskFile))
            requestClose(FILE_ERROR, "Error while reading disk file!");
    }
    void in(Memory* mem, dword in)
    {
        if(m_InputMode == IOInputMode::INSTRUCTION)
        {
            switch (in)
            {
            case IO_DRIVE_READ_BLOCKS:
                loadBlock(m_io_BlockAdr, m_io_BlockCount, mem, m_io_MemAdr);
                break;
            case IO_DRIVE_WRITE_BLOCKS:
                storeBlock(m_io_BlockAdr, m_io_BlockCount, mem, m_io_MemAdr);
                break;
            case IO_DRIVE_SET_BLOCK_ADR:
            case IO_DRIVE_SET_BLOCK_COUNT:
            case IO_DRIVE_SET_MEM_ADR:
                m_InputMode = IOInputMode::ARGUMENT;    
                break;
            case IO_DRIVE_STATUS:
            case IO_DRIVE_GET_SIZE:
                break;        
            default:
                requestClose(IO_INVALID_OPERATION, "INVALID DRIVE IO OPERATION");
                break;
            }
            m_io_LastInstruction = in;
            return;
        }
        if(m_InputMode == IOInputMode::ARGUMENT)
        {
            switch (m_io_LastInstruction)
            {
            case IO_DRIVE_SET_BLOCK_ADR:
                m_io_BlockAdr = in;
                break;
            case IO_DRIVE_SET_BLOCK_COUNT:
                m_io_BlockCount = in;
                break;
            case IO_DRIVE_SET_MEM_ADR:
                m_io_MemAdr = in;
                break;
            default:
                requestClose(IO_INVALID_OPERATION, "INVALID DRIVE IO OPERATION");
                break;
            }
            m_InputMode == IOInputMode::INSTRUCTION;
        }
    }
    dword out()
    {
        switch(m_io_LastInstruction)
        {
            case IO_DRIVE_STATUS:
                return 0; // TODO: implement DRIVE_STATUS
            case IO_DRIVE_GET_SIZE:
                return m_Size;
        }
    }
};

enum ScreenMode { TEXT_MODE, PXL_MODE };
class ScreenDevice
{
private:

    dword*      m_ScrBuffer;
    IOInputMode m_InputMode;
    dword       m_io_cx; // Cursor position x in text mode
    dword       m_io_cy; // Cursor position y in text mode
    dword       m_io_LastInstruction;
    ScreenMode  m_Mode;

public:
    ScreenDevice(Memory* mem)
    {
        debug("Creating screen");
        m_ScrBuffer = &(mem->at<dword>(SCR_BUFFER));
        m_InputMode = IOInputMode::INSTRUCTION;
        for(uint i = 0; i < SCR_HEIGHT * SCR_WIDTH; i++)
            m_ScrBuffer[i] = 0;
        debug("Screen created");
    }
    dword* buffer()
    {
        return m_ScrBuffer;
    }
    void setPixelAt(dword x, dword y, dword color)
    {
        m_ScrBuffer[x + y * SCR_WIDTH] = color;
    }
    void update()
    {
        refreshScr(mainWin, m_ScrBuffer);
    }
    void in(dword in)
    {
        if(m_InputMode == IOInputMode::INSTRUCTION)
        {
            switch (in)
            {
            case IO_SCR_SET_TEXT_MODE:
                m_Mode = ScreenMode::TEXT_MODE;
                break;
            case IO_SCR_SET_PXL_MODE:
                m_Mode = ScreenMode::PXL_MODE;
                break;
            case IO_SCR_SET_CURSOR_X:
            case IO_SCR_SET_CURSOR_Y:
                break;
            case IO_SCR_FORCE_REFRESH:
                refreshScr(mainWin, m_ScrBuffer, true);
                break;
            default:
                requestClose(IO_INVALID_OPERATION, "Invalid screen IO instruction");
                break;
            }
            m_io_LastInstruction = in;
            return;
        }
        if(m_InputMode == IOInputMode::ARGUMENT)
        {
            switch (m_io_LastInstruction)
            {
            case IO_SCR_SET_CURSOR_X:
                m_io_cx = in;
                break;
            case IO_SCR_SET_CURSOR_Y:
                m_io_cy = in;
                break;
            default:
                requestClose(IO_INVALID_OPERATION, "Invalid screen IO argument");
                break;
            }
        }
    }
};

#include <reg_internal.h>
#include <cpu_internal.h>


namespace devices
{
    extern CPU* cpu;
    void init(dword memSize, dword diskSizeIfNotPresentInBlocks);
    void update();
    void exit();
}


#endif
