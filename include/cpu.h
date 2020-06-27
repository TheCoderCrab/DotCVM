//This file should only be included through devices.h, otherwise compile errors.
#ifdef CPU_H
#error cpu.h sould only be included from devices, use '#include <devices.h>' instead.
#endif

// This should be defined in devices.h
#ifndef CPU_INCLUDE_ERRORS_PROTECTION // This is so that the IDE will shut up about classes not being defined
#include <devices.h>
#endif

#ifndef CPU_H
#define CPU_H

struct Reg8
{
    union
    {
        uint8_t      uval8;
        int8_t        val8;
    };
    Reg8 operator=(uint8_t val)
    {
        return {val};
    }
};

struct Reg16
{
    union
    {
        uint16_t    uval16;
        int16_t      val16;
        uint8_t      uval8;
        int8_t        val8;
        uint8_t  ubytes[2];
        int8_t    bytes[2];
    };
    Reg16 operator=(uint16_t val)
    {
        return {val};
    }
};

struct Reg32
{
    union
    {
        uint32_t    uval32;
        int32_t      val32;
        uint16_t    uval16;
        int16_t      val16;
        uint8_t      uval8;
        int8_t        val8;
        uint16_t uwords[2];
        int16_t   words[2];
        uint8_t  ubytes[4];
        int8_t    bytes[4];
    };
};

struct Reg48
{
    Reg32 l32;
    Reg16 h16;
    Reg48 operator=(uint64_t val)
    {
        return {{(uint32_t) val}, {(uint16_t)(val >> 32)}};
    }
};

class CPU : public Device
{
private:
    // General purpose: 0X0XH
    Reg32     a;
    Reg32     b;
    Reg32     c;
    Reg32     d;


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


    // Security: 6X0XH
    Reg8    cpl; // Current privilege level


    // Devices
    Memory*  m_Mem;
    Drive* m_Drive;
    Screen*  m_Scr;

public:

    CPU(Memory* mem, Drive* drive, Screen* scr)
    {
        m_Mem   =   mem;
        m_Drive = drive;
        m_Scr   =   scr;
    }
    virtual ~CPU()
    {
        delete m_Mem;
        delete m_Drive;
        delete m_Scr;

    }
    void jmp(uint32_t address)
    {
        ip.uval32 = address;
    }
    void call(uint32_t address)
    {
        rp.uval32 = ip.uval32;
        jmp(address);
    }
    void ret()
    {
        jmp(rp.uval32);
    }
    void iret()
    {
        jmp(irp.uval32);
    }
    void push8(uint8_t val)
    {
        (*m_Mem)[sp.uval32] = val;
        sp.uval32++;
    }
    void interrupt(uint8_t interruptCode)
    {
        push8(interruptCode);
        irp.uval32 = ip.uval32;
        jmp(ihp.uval32);
    }
    virtual void update() override
    {
        debug("mem update");
        m_Mem->update();
        debug("drive update");
        m_Drive->update();
        debug("scr update");
        m_Scr->update();
    }
    virtual void exit() override
    {
        debug("mem exit");
        m_Mem->exit();
        debug("drive exit");
        m_Drive->exit();
        debug("scr exit");
        m_Scr->exit();
    }
    Memory& mem() {return *m_Mem; }
    Drive& drive() {return *m_Drive; }
    Screen& screen() {return *m_Scr; }

};

#endif
