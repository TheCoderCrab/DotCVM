#include <DotCVM/devices/instructions.h>


Argument::Argument(CPU* cpu, ArgumentType type, dword rawVal)
{
    m_cpu = cpu;
    m_Type = type;
    m_RawVal = rawVal;
}

byte&   Argument::memByte()
{
    return m_cpu->mem().at<byte>(m_RawVal);
}
word&   Argument::memWord()
{
    return m_cpu->mem().at<word>(m_RawVal);
}
dword&  Argument::memDword()
{
    return m_cpu->mem().at<dword>(m_RawVal);
}

Reg8&   Argument::reg8()
{

}
Reg16&  Argument::reg16()
{

}
Reg32&  Argument::reg32()
{

}

dword   Argument::literal()
{
    return m_RawVal;
}