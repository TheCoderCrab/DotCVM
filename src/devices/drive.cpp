#include <DotCVM/devices/devices.h>
#include <DotCVM/gui/window.h>
#include <DotCVM/utils/file_utils.h>
#include <DotCVM/utils/log.h>
#include <DotCVM/utils/err_code.h>
#include <stdio.h>


Drive::Drive(dword sizeIfNotPresentInBlocks)
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
Drive::~Drive()
{
    fclose(m_DiskFile);
}
void Drive::loadBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress)
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
void Drive::storeBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress)
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
void Drive::in(Memory* mem, dword in)
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
        m_InputMode = IOInputMode::INSTRUCTION;
    }
}
dword Drive::out()
{
    switch(m_io_LastInstruction)
    {
        case IO_DRIVE_STATUS:
            return 0; // TODO: implement DRIVE_STATUS
        case IO_DRIVE_GET_SIZE:
            return m_Size;
        default:
            requestClose(IO_INVALID_OPERATION, "IO fail");
            return 0;
    }
}
