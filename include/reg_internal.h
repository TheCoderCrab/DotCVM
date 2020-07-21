#ifndef REG_H
#define REG_H

struct Reg8
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
    void load(Memory* mem, address adr)
    {
        uval8 = (*mem)[adr];
    }
    void store(Memory* mem, address adr)
    {
        (*mem)[adr] = uval8;
    }
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
    void load(Memory* mem, address adr)
    {
        uval16 = mem->at<uint16_t>(adr);
    }
    void store(Memory* mem, address adr)
    {
        mem->at<uint16_t>(adr) = uval16;
    }
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
    void load(Memory* mem, address adr)
    {
        uval32 = mem->at<uint32_t>(adr);
    }
    void store(Memory* mem, address adr)
    {
        mem->at<uint32_t>(adr) = uval32;
    }
};

struct Reg48
{
    Reg32 l32;
    Reg16 h16;
    void load(Memory* mem, address adr)
    {
        h16.load(mem, adr);
        l32.load(mem, adr + 2);
    }
    void store(Memory* mem, address adr)
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

#endif