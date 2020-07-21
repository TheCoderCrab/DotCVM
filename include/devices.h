// TODO: make io invalid operations set iof flag instead of requestClose

#ifndef DEVICES_H
#define DEVICES_H

#include <log.h>
#include <file_utils.h>
#include <fstream>
#include <app_main.h>
#include <optional>
#include <maths.h>
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
#define MEM_SIZE        50 * 1024 * 1024
#define DISK_SIZE       (100 * 1024 * 1024) / BLOCK_SIZE

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
    }
};

struct Reg
{
    virtual void load(Memory* mem, uint32_t address) = 0;
    virtual void store(Memory* mem, uint32_t address) = 0;
};

struct Reg8 : public Reg
{
    union
    {
        byte      uval8;
        sbyte      val8;
    };
    Reg8()
    {
        val8 = 0;
    }
    Reg8(byte val)
    {
        val8 = val;
    }
    Reg8 operator=(byte val)
    {
        return val;
    }
    operator byte()
    {
        return uval8;
    }
    bool operator[](byte offset)
    {
        return (uval8 << (7 - offset)) >> 7;
    }
    void operator()(byte offset, bool val)
    {
        uval8 |= val << offset;
    }
    void load(Memory* mem, address adr) override
    {
        uval8 = (*mem)[adr];
    }
    void store(Memory* mem, address adr) override
    {
        (*mem)[adr] = uval8;
    }
};

struct Reg16 : public Reg
{
    union
    {
        word       uval16;
        sword       val16;
        byte        uval8;
        sbyte        val8;
        byte    ubytes[2];
        sbyte    bytes[2];
    };
    Reg16()
    {
        val16 = 0;
    }
    Reg16(word val)
    {
        val16 = val;
    }
    Reg16 operator=(word val)
    {
        return val;
    }
    operator word()
    {
        return uval16;
    }
    bool operator[](byte offset)
    {
        return (uval16 << (15 - offset)) >> 15;
    }
    void operator()(byte offset, bool val)
    {
        uval16 |= val << offset;
    }
    void load(Memory* mem, address adr) override
    {
        uval16 = mem->at<uint16_t>(adr);
    }
    void store(Memory* mem, address adr) override
    {
        mem->at<uint16_t>(adr) = uval16;
    }
};

struct Reg32 : public Reg
{
    union
    {
        dword      uval32;
        sdword      val32;
        float        fval;
        word       uval16;
        sword       val16;
        byte        uval8;
        sbyte        val8;
        word    uwords[2];
        sword    words[2];
        byte    ubytes[4];
        sbyte    bytes[4];
    };
    Reg32()
    {
        val32 = 0;
    }
    Reg32(dword val)
    {
        val32 = val;
    }
    Reg32 operator=(dword val)
    {
        return val;
    }
    Reg32 operator+=(dword val)
    {
        return uval32 + val;
    }
    Reg32 operator-=(dword val)
    {
        return uval32 - val;
    }
    operator dword()
    {
        return uval32;
    }
    void operator()(byte offset, bool val)
    {
        uval32 |= val << offset;
    }
    bool operator[](byte offset)
    {
        return (uval32 << (31 - offset)) >> 31;
    }
    void load(Memory* mem, address adr) override
    {
        uval32 = mem->at<uint32_t>(adr);
    }
    void store(Memory* mem, address adr) override
    {
        mem->at<uint32_t>(adr) = uval32;
    }
};

struct Reg48 : public Reg
{
    Reg32 l32;
    Reg16 h16;
    void load(Memory* mem, address adr) override
    {
        h16.load(mem, adr);
        l32.load(mem, adr + 2);
    }
    void store(Memory* mem, address adr) override
    {
        h16.store(mem, adr);
        l32.store(mem, adr + 2);
    }
    bool operator[](byte offset)
    {
        return offset >= 32 ? h16[offset - 32] : l32[offset];
    }
    void operator()(byte offset, bool val)
    {
        if(offset >= 32)
            h16(offset - 32, val);
        else
            l32(offset, val);
    }
};

class CPU
{
private:
    // General purpose: 0X0XH
    Reg32     a;
    Reg32     b;
    Reg32     c;
    Reg32     d;
    Reg32     r; // Result
    Reg32     f; // Flags
    Reg32     t; // Temporary register
    Reg32     u; // Temporary register


    // Memory management: 1X0XH
    Reg32    sp; // Stack pointer
    Reg48  gdtr; // GDT Pointer register
    Reg16    ds; // DATA segment descriptor
    Reg16    ss; // STACK segment descriptor
    Reg16    cs; // CODE segment descriptor


    // Interrupts: 2X0XH
    Reg32   irp; // Interrupt return pointer
    Reg32   ihp; // Interrupt handler pointer


    // Execution flow: 3X0XH
    Reg32    rp; // Return pointer
    Reg32    ip; // Instruction pointer


    // Control: 4X0XH;
    Reg32   cr0;
    Reg32   cr1;
    Reg32   cr2;


    // Free reg: 5X0XH
    Reg32    fa;
    Reg32    fb;
    Reg32    fc;
    Reg32    fd;
    Reg32    fr; // Result


    // Security: 6X0XH
    Reg8    cpl; // Current privilege level


    // Devices
    Memory*  m_Mem;
    Drive* m_Drive;
    ScreenDevice*  m_Scr;

public:

    CPU(Memory* mem, Drive* drive, ScreenDevice* scr)
    {
        debug("Creating CPU");
        m_Mem   =   mem;
        m_Drive = drive;
        m_Scr   =   scr;
        debug("CPU created");
    }
    ~CPU()
    {
        delete m_Mem;
        delete m_Drive;
        delete m_Scr;

    }

    void update()
    {
        m_Scr->update();
    }
    Memory& mem() {return *m_Mem; }
    Drive& drive() {return *m_Drive; }
    ScreenDevice& screen() {return *m_Scr; }

    // Instructions
    // Returns wether or not register ip should be set to next instruction,
    // true = change reg ip, false = don't

    // Misc: 0XXXH

    bool cpuinf() // 00XXH
    {
        if(a == 0) // cpu signature
        {
            a = utils::bytesToInt('C', 't', 'o', 'D');
            b = utils::bytesToInt('o', 'D', 'M', 'V');
            c = utils::bytesToInt('o', 'D', 'C', 't');
            d = utils::bytesToInt('M', 'V', 'C', 't');
        }
        return true;
    }

    // Data management: 1XXXH
    // set: 10XXH
    template<typename T = dword> bool set(T& dest, T val)
     {
         dest = val;
         return true;
     }
    // push: 11XXH
    template<typename T = dword> bool push(T val)
    {
        mem().at<T>(sp) = val;
        sp += sizeof(T);
        return true;
    }
    // pop: 12XXH
    template<typename T = dword> bool pop(T& val)
    {
        sp -= sizeof(T);
        val = mem().at<T>(sp);
        return true;
    }
    // lgdtr: 13XXH
    bool lgdtr(address adr)
    {
        gdtr.load(m_Mem, adr);
        return true;
    }
    // sgdtr: 14XXH
    bool sgdtr(address adr)
    {
        gdtr.store(m_Mem, adr);
        return true;
    }
    // lds: 15XXH
    bool lds(word val)
    {
        ds = val;
        return true;
    }
    // lcs: 16XXH
    bool lcs(word val)
    {
        cs = val;
        return true;
    }
    // lss: 17XXH
    bool lss(word val)
    {
        ss = val;
        return true;
    }
    // sds: 18XXH
    bool sds(word& ctnr)
    {
        ctnr = ds;
        return true;
    }
    // scs: 19XXH
    bool scs(word& ctnr)
    {
        ctnr = cs;
        return true;
    }
    // sss: 18XXH
    bool sss(word& ctnr)
    {
        ctnr = ss;
        return true;
    }

    //Interrupts: 2XXXH
    // lihp: 21XXH
    bool lihp(dword val)
    {
        ihp = val;
        return true;
    }
    // sihp: 22XXH
    bool sihp(dword& ctnr)
    {
        ctnr = ihp;
        return true;
    }
    // int: 23XXH
    bool intr(byte intCode)
    {
        push(a);
        push(b);
        push(c);
        push(d);
        push(r);
        push(f);

        push(fa);
        push(fb);
        push(fc);
        push(fd);
        push(fr);

        push(ds);
        push(cs);
        push(ss);

        push(cpl);

        push(rp);

        push(intCode);

        irp = ip; // ALERT: Next instruction after ip actually, not ip itself

        cs  = 0;
        ds  = 0;
        ss  = 0;
        cpl = 0;

        ip  = ihp;

        return false;
    }
    // iret: 24XXH
    bool iret()
    {
        pop(t); // Interrupt code
        pop(rp);
        pop(cpl);
        pop(t); // ss
        pop(cs);
        pop(u); // ds

        pop(fr);
        pop(fd);
        pop(fc);
        pop(fb);
        pop(fa);

        pop(f);
        pop(r);
        pop(d);
        pop(c);
        pop(b);
        pop(a);

        ds = u;
        ss = t;

        ip = irp;
        return false;
    }

    // Arithmetic and logic: 3XXXH
    // not: 30XXH
    bool not_(dword& result, dword operand)
    {
        result = !(operand);
        return true;
    }
    bool not_(dword& operand)
    {
        operand = !operand;
        return true;
    }
    // add: 31XXH
    bool add(dword& op1, dword op2)
    {
        op1 += op2;
        return true;
    }
    // sub: 32XXH
    bool sub(dword& op1, dword op2)
    {
        op1 -= op2;
        return true;
    }
    // mul: 33XXH
    bool mul(dword& op1, dword op2)
    {
        op1 *= op2;
        return true;
    }
    // div: 34XXH
    bool div(dword& op1, dword op2)
    {
        op1 /= op2;
        return true;
    }
    // fadd: 31XXH
    bool fadd(float& op1, float op2)
    {
        op1 += op2;
        return true;
    }
    // fsub: 32XXH
    bool fsub(float& op1, float op2)
    {
        op1 -= op2;
        return true;
    }
    // fmul: 33XXH
    bool fmul(float& op1, float op2)
    {
        op1 *= op2;
        return true;
    }
    // fdiv: 34XXH
    bool fdiv(float& op1, float op2)
    {
        op1 /= op2;
        return true;
    }

    // Execution flow: 4XXXH
    // jmp: 40XXH
    bool jmp(address adr)
    {
        ip = adr;
        return false;
    }
    // call: 41XXH
    bool call(address adr)
    {
        rp = ip; // ALERT: Make so that it returns to the next instruction
        ip = adr;
        return false;
    }
    // cmp: 42XXH | fcmp: 43XXH | ucmp: 44XXH
    template<typename T> bool cmp(T op1, T op2)
    {
        f(0, op1 > op2);
        f(1, a < b);
        return true;
    }
    // je: 46XXH | jz: 47XXH | jg: 48XXH | jl: 49XXH | jeg: 4AXXH | jel: 4BXXH | jiof: 4CXXH
    bool joc(address adr, bool c) // Jump on condition
    {
        if(c)
            ip = adr;
        return false;
    }
    // ret: 4DXXH
    bool ret()
    {
        ip = rp;
        return false;
    }
    // hlt: 4EXXH
    bool hlt()
    {
        return false; // by returning false ip won't change so we hang here.
    }
    // sleep: 4FXXH
    bool sleep()
    {
        // TODO: implemnt sleep
        return true;
    }
    // Control: 5XXXH
    //Not defined yet

    // Security: 6XXXH
    // lcpl: 60XXH
    bool lcpl(byte ncpl)
    {
        // TODO: check for 0 <= cpl <= 3
        cpl = ncpl;
        return true;
    }
    // scpl: 61XXH
    bool scpl(byte& ctn)
    {
        ctn = cpl;
        return true;
    }
    // Arithmetic and logic 2: 7XXH
    bool intg(dword& op1, float op2)
    {
        op1 = (dword) op2;
        return true;
    }
    bool ifloat(float& op1, dword op2)
    {
        op1 = (float) op2;
        return true;
    }
    // IO: 8XXXH
    // out: 80XXH
    bool out(dword adr, dword code)
    {
        switch(adr)
        {
            case IO_DRIVE_ADR:
                m_Drive->in(m_Mem, code);
                break;
            case IO_SCR_ADR:
                m_Scr->in(code);
            default:
                requestClose(IO_INVALID_OPERATION, "Invalid IO address");
                break;
        }
        // TODO: implement out (Memory and screen)
        return true;
    }
    // in: 81XXH
    bool in(dword adr, dword& ret)
    {
        switch (adr)
        {
        case IO_DRIVE_ADR:
            ret = m_Drive->out();
            break;
        case IO_MEM_ADR:
            ret = m_Mem->out();
        default:
            requestClose(IO_INVALID_OPERATION, "Invalid IO address");
            break;
        }
        // TODO: implement in (Memory and screen)
        return true;
    }
};

namespace devices
{
    extern CPU* cpu;
    void init(dword memSize, dword diskSizeIfNotPresentInBlocks);
    void update();
    void exit();
}


#endif
