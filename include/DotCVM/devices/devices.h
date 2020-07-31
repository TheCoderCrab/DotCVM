// TODO: make io invalid operations set iof flag instead of requestClose

#ifndef DEVICES_H
#define DEVICES_H

#include <DotCVM/utils/types.h>
#include <DotCVM/devices/io.h>
#include <stdio.h>

#define BLOCK_SIZE      512
#define DISK_FILE       "disk.dat"
#define DISK_SIZE       (100 * 1024 * 1024) / BLOCK_SIZE

#define MEM_SIZE        50 * 1024 * 1024

#define SCR_BUFFER      0xA0000
#define SCR_WIDTH       720
#define SCR_HEIGHT      480

class CPU;

class Keyboard
{
private:
    dword m_LastKey;
public:
    Keyboard();
    void press(CPU* cpu, dword keyCode);
    void release(CPU* cpu, dword keyCode);
    dword out();
};

enum MouseEventType { NONE, BUTTON_PRESS, BUTTON_RELEASE, MOTION };

class Mouse
{
private:
    MouseEventType m_LastEventType;
    dword          m_ButtonNumber;
    word           m_PointerX, m_PointerY;
public:
    Mouse();
    void move(CPU* cpu, dword x, dword y);
    void press(CPU* cpu, dword buttonNum);
    void release(CPU* cpu, dword buttonNum);
    dword out();   
};

class DebugConsole
{
public:
    void in(dword in);
};

class Memory
{
private:
    data* m_Data;
    dword m_Size;
public:
    Memory(uint32_t size);
    ~Memory();
    template<typename T> T& at(address adr) { return (T&)(m_Data[adr]); }
    byte& operator[](address adr);
    dword size();
    dword out();
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
    Drive(dword sizeIfNotPresentInBlocks);
    ~Drive();
    void loadBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress);
    void storeBlock(address blockAddress, dword blockCount, Memory* mem, address memAddress);
    void in(Memory* mem, dword in);
    dword out();
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
    ScreenDevice(Memory* mem);
    dword* buffer();
    void update();
    void in(dword in);
};

#include <DotCVM/devices/cpu_internal.h>

namespace devices
{
    extern CPU* cpu;
    void init(dword memSize, dword diskSizeIfNotPresentInBlocks);
    void update();
    void exit();
}


#endif
