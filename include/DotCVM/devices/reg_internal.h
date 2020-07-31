#ifndef REG_INTERNAL_H
#define REG_INTERNAL_H


#include <DotCVM/utils/types.h>
#include <DotCVM/devices/instructions.h>

struct Reg8
{
    union
    {
        byte      uval8;
        sbyte      val8;
    };
    Reg8();
    Reg8(byte val);
    Reg8 operator=(byte val);
    Reg8 operator+=(byte val);
    Reg8 operator-=(byte val);
    Reg8 operator*=(byte val);
    Reg8 operator/=(byte val);
    operator byte();
    bool operator[](byte offset);
    void operator()(byte offset, bool val);
    void load(Memory* mem, address adr);
    void store(Memory* mem, address adr);
};

struct Reg16
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
    Reg16();
    Reg16(word val);
    Reg16 operator=(word val);
    Reg16 operator+=(word val);
    Reg16 operator-=(word val);
    Reg16 operator*=(word val);
    Reg16 operator/=(word val);
    operator word();
    bool operator[](byte offset);
    void operator()(byte offset, bool val);
    void load(Memory* mem, address adr);
    void store(Memory* mem, address adr);
};

struct Reg32
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
    Reg32();
    Reg32(dword val);
    Reg32 operator=(dword val);
    Reg32 operator+=(dword val);
    Reg32 operator-=(dword val);
    Reg32 operator*=(dword val);
    Reg32 operator/=(dword val); 
    operator dword();
    void operator()(byte offset, bool val);
    bool operator[](byte offset);
    void load(Memory* mem, address adr);
    void store(Memory* mem, address adr);
};

struct Reg48
{
    Reg32 l32;
    Reg16 h16;
    void load(Memory* mem, address adr);
    void store(Memory* mem, address adr);
    bool operator[](byte offset);
    void operator()(byte offset, bool val);
};

#endif