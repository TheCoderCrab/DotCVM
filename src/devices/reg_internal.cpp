#include <DotCVM/devices/devices.h>


// Reg8

Reg8::Reg8()
{
    val8 = 0;
}
Reg8::Reg8(byte val)
{
    val8 = val;
}
Reg8 Reg8::operator=(byte val)
{
    return val;
}
Reg8 Reg8::operator+=(byte val)
{
    return Reg8(uval8 + val);
}
Reg8 Reg8::operator-=(byte val)
{
    return Reg8(uval8 - val);
}
Reg8 Reg8::operator*=(byte val)
{
    return Reg8(uval8 * val);
}
Reg8 Reg8::operator/=(byte val)
{
    return Reg8(uval8 / val);
}

Reg8::operator byte()
{
    return uval8;
}
bool Reg8::operator[](byte offset)
{
    return (uval8 << (7 - offset)) >> 7;
}
void Reg8::operator()(byte offset, bool val)
{
    uval8 |= val << offset;
}
void Reg8::load(Memory* mem, address adr)
{
    uval8 = (*mem)[adr];
}
void Reg8::store(Memory* mem, address adr)
{
    (*mem)[adr] = uval8;
}


// Reg16

Reg16::Reg16()
{
    val16 = 0;
}
Reg16::Reg16(word val)
{
    val16 = val;
}
Reg16 Reg16::operator=(word val)
{
    return val;
}

Reg16 Reg16::operator+=(word val)
{
    return Reg16(uval16 + val);
}
Reg16 Reg16::operator-=(word val)
{
    return Reg16(uval16 - val);
}
Reg16 Reg16::operator*=(word val)
{
    return Reg16(uval16 * val);
}
Reg16 Reg16::operator/=(word val)
{
    return Reg16(uval16 / val);
}

Reg16::operator word()
{
    return uval16;
}
bool Reg16::operator[](byte offset)
{
    return (uval16 << (15 - offset)) >> 15;
}
void Reg16::operator()(byte offset, bool val)
{
    uval16 |= val << offset;
}
void Reg16::load(Memory* mem, address adr)
{
    uval16 = mem->at<uint16_t>(adr);
}
void Reg16::store(Memory* mem, address adr)
{
    mem->at<uint16_t>(adr) = uval16;
}


// Reg32

Reg32::Reg32()
{
    val32 = 0;
}
Reg32::Reg32(dword val)
{
    val32 = val;
}
Reg32 Reg32::operator=(dword val)
{
    return val;
}
Reg32 Reg32::operator+=(dword val)
{
    return uval32 + val;
}
Reg32 Reg32::operator-=(dword val)
{
    return uval32 - val;
}
Reg32 Reg32::operator*=(dword val)
{
    return uval32 * val;
}
Reg32 Reg32::operator/=(dword val)
{
    return uval32 / val;
}

Reg32::operator dword()
{
    return uval32;
}

void Reg32::operator()(byte offset, bool val)
{
    uval32 |= val << offset;
}
bool Reg32::operator[](byte offset)
{
    return (uval32 << (31 - offset)) >> 31;
}
void Reg32::load(Memory* mem, address adr)
{
    uval32 = mem->at<uint32_t>(adr);
}
void Reg32::store(Memory* mem, address adr)
{
    mem->at<uint32_t>(adr) = uval32;
}


// Reg48

void Reg48::load(Memory* mem, address adr)
{
    h16.load(mem, adr);
    l32.load(mem, adr + 2);
}
void Reg48::store(Memory* mem, address adr)
{
    h16.store(mem, adr);
    l32.store(mem, adr + 2);
}
bool Reg48::operator[](byte offset)
{
    return offset >= 32 ? h16[offset - 32] : l32[offset];
}
void Reg48::operator()(byte offset, bool val)
{
    if(offset >= 32)
        h16(offset - 32, val);
    else
        l32(offset, val);
}
