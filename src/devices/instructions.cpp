#include <DotCVM/devices/devices.h>
#include <DotCVM/devices/instructions.h>
#include <DotCVM/utils/log.h>

Argument::Argument(CPU* cpu, ArgumentType type, dword rawVal, byte argPosition)
{
    m_cpu = cpu;
    m_Type = type;
    m_RawVal = rawVal;
    m_ArgPosition = argPosition;
    m_reg8 = nullptr;
    m_reg16 = nullptr;
    m_reg32 = nullptr;
}

Argument::Argument(Reg8& reg)
{
    m_Type = ArgumentType::REG;
    m_reg8 = &reg;
    m_reg16 = nullptr;
    m_reg32 = nullptr;   
}
Argument::Argument(Reg16& reg)
{
    m_Type = ArgumentType::REG;
    m_reg8 = nullptr;
    m_reg16 = &reg;
    m_reg32 = nullptr;
}
Argument::Argument(Reg32& reg)
{
    m_Type = ArgumentType::REG;
    m_reg8 = nullptr;
    m_reg16 = nullptr;
    m_reg32 = &reg; 
}

byte&   Argument::memByte()
{
    return m_cpu->mem().at<byte>(m_ArgPosition == 0 ? m_cpu->memr0 : m_cpu->memr1);
}
word&   Argument::memWord()
{
    return m_cpu->mem().at<word>(m_ArgPosition == 0 ? m_cpu->memr0 : m_cpu->memr1);
}
dword&  Argument::memDword()
{
    return m_cpu->mem().at<dword>(m_ArgPosition == 0 ? m_cpu->memr0 : m_cpu->memr1);
}

Reg8&   Argument::reg8()
{
    if(m_reg8 != nullptr)
        return *m_reg8;
    return m_cpu->reg8(m_RawValBytes[0]);
}
Reg16&  Argument::reg16()
{
    if(m_reg16 != nullptr)
        return *m_reg16;
    return m_cpu->reg16(m_RawValBytes[0]);
}
Reg32&  Argument::reg32()
{
    if(m_reg32 != nullptr)
        return *m_reg32;
    return m_cpu->reg32(m_RawValBytes[0]);
}

dword   Argument::literal()
{
    return m_RawVal;
}

void Argument::operator=(Reg16 reg)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() = reg;
        else if(ARG_MEM16(m_Type))
            memWord() = reg;
        else if(ARG_MEM32(m_Type))
            memDword() = reg;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() = reg;
                break;
            case 16:
                reg16() = reg;
                break;
            case 32:
                reg32() = reg;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}
void Argument::operator=(Reg32 reg)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() = reg;
        else if(ARG_MEM16(m_Type))
            memWord() = reg;
        else if(ARG_MEM32(m_Type))
            memDword() = reg;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() = reg;
                break;
            case 16:
                reg16() = reg;
                break;
            case 32:
                reg32() = reg;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}

void Argument::operator=(dword val)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() = val;
        else if(ARG_MEM16(m_Type))
            memWord() = val;
        else if(ARG_MEM32(m_Type))
            memDword() = val;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() = val;
                break;
            case 16:
                reg16() = val;
                break;
            case 32:
                reg32() = val;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}

void Argument::operator+=(dword val)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() += val;
        else if(ARG_MEM16(m_Type))
            memWord() += val;
        else if(ARG_MEM32(m_Type))
            memDword() += val;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() += val;
                break;
            case 16:
                reg16() += val;
                break;
            case 32:
                reg32() += val;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}
void Argument::operator-=(dword val)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() -= val;
        else if(ARG_MEM16(m_Type))
            memWord() -= val;
        else if(ARG_MEM32(m_Type))
            memDword() -= val;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() -= val;
                break;
            case 16:
                reg16() -= val;
                break;
            case 32:
                reg32() -= val;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}
void Argument::operator*=(dword val)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() *= val;
        else if(ARG_MEM16(m_Type))
            memWord() *= val;
        else if(ARG_MEM32(m_Type))
            memDword() *= val;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() *= val;
                break;
            case 16:
                reg16() *= val;
                break;
            case 32:
                reg32() *= val;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}
void Argument::operator/=(dword val)
{
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() /= val;
        else if(ARG_MEM16(m_Type))
            memWord() /= val;
        else if(ARG_MEM32(m_Type))
            memDword() /= val;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() /= val;
                break;
            case 16:
                reg16() /= val;
                break;
            case 32:
                reg32() /= val;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}

Argument::operator dword()
{
    if(ARG_LITERAL(m_Type))
        return literal();
    if(ARG_MEM8(m_Type))
        return memByte();
    if(ARG_MEM16(m_Type))
        return memWord();
    if(ARG_MEM32(m_Type))
        return memDword();
    if(ARG_REG(m_Type))
    {
        switch (m_cpu->regSize(m_RawValBytes[0]))
        {
        case 8:
            return reg8();
        case 16:
            return reg16();
        case 32:
            return reg32();
        default:
            return -1;
        }
    }   
}

float Argument::asFloat()
{
    if(m_Type == ArgumentType::DISABLED)
        return 0.0f;
    if(m_Type == ArgumentType::LITERAL)
        return m_floatRawVal;
    if(ARG_MEM(m_Type))
        return m_cpu->mem().at<float>(m_ArgPosition == 0 ? m_cpu->memr0 : m_cpu->memr1);
}

void Argument::operator=(float valin)
{
    union
    {
        float fval;
        dword dwordv;
        word words[2];
        byte bytes[4];
    };
    fval = valin;
    if(ARG_WRITABLE(m_Type))
    {
        if(ARG_MEM8(m_Type))
            memByte() = bytes[3];
        else if(ARG_MEM16(m_Type))
            memWord() = words[1];
        else if(ARG_MEM32(m_Type))
            memDword() = dwordv;
        else if(ARG_REG(m_Type))
            switch (m_cpu->regSize(m_RawValBytes[0]))
            {
            case 8:
                reg8() = bytes[3];
                break;
            case 16:
                reg16() = words[1];
                break;
            case 32:
                reg32() = dwordv;
                break;
            }
    }
    else
        err("Can't write to that type of argument");
}

