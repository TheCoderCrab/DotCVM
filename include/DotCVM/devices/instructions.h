#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/types.h>

#define INSTRUCTION_CPUINFO     0x00

#define INSTRUCTION_SET         0x10
#define INSTRUCTION_PUSH        0x11
#define INSTRUCTION_POP         0x12
#define INSTRUCTION_LGDTR       0x13
#define INSTRUCTION_SGDTR       0x14
#define INSTRUCTION_LDS         0x15
#define INSTRUCTION_LCS         0x16
#define INSTRUCTION_LSS         0x17
#define INSTRUCTION_SDS         0x18
#define INSTRUCTION_SCS         0x19
#define INSTRUCTION_SSS         0x1A

#define INSTRUCTION_LIHP        0x20
#define INSTRUCTION_SIHP        0x21
#define INSTRUCTION_INT         0x22
#define INSTRUCTION_IRET        0x23

#define INSTRUCTION_NOT         0x30
#define INSTRUCTION_OR          0x31
#define INSTRUCTION_AND         0x32
#define INSTRUCTION_XOR         0x33
#define INSTRUCTION_NOR         0x34
#define INSTRUCTION_NAND        0x35
#define INSTRUCTION_XNOR        0x36
#define INSTRUCTION_ADD         0x37
#define INSTRUCTION_FADD        0x38
#define INSTRUCTION_SUB         0x39
#define INSTRUCTION_FSUB        0x3A
#define INSTRUCTION_MUL         0x3B
#define INSTRUCTION_FMUL        0x3C
#define INSTRUCTION_DIV         0x3D
#define INSTRUCTION_FDIV        0x3E

#define INSTRUCTION_JMP         0x40
#define INSTRUCTION_CALL        0x41
#define INSTRUCTION_CMP         0x42
#define INSTRUCTION_FCMP        0x43
#define INSTRUCTION_UCMP        0x44
#define INSTRUCTION_JE          0x45
#define INSTRUCTION_JZ          0x46
#define INSTRUCTION_JG          0x47
#define INSTRUCTION_JL          0x48
#define INSTRUCTION_JEG         0x49
#define INSTRUCTION_JEL         0x4A
#define INSTRUCTION_JIOF        0x4B
#define INSTRUCTION_RET         0x4C
#define INSTRUCTION_HLT         0x4D
#define INSTRUCTION_SLEEP       0x4E

#define INSTRUCTIONS_ctrl // TODO: add control inbstructions

#define INSTRUCTION_LCPL        0x60
#define INSTRUCTION_SCPL        0x61

#define INSTRUCTION_INTG        0x70
#define INSTRUCTION_FLOAT       0x71

#define INSTRUCTION_OUT         0x80
#define INSTRUCTION_IN          0x81

#define INSTRUCTIONS_flags // TODO: add flag instructions

#define ARG_WRITABLE(x)     ((dword) ArgumentType::MEM8 <= (dword) x && (dword) x <= ArgumentType::REG)

enum ArgumentType { DISABLED, 
                    LITERAL,
                    MEM8, MEM16, MEM32,
                    REG };

struct Argument
{
private:
    dword        m_RawVal;
    ArgumentType m_Type;
    CPU*         m_cpu;
public:
    Argument(CPU* cpu, ArgumentType type, dword val);
    byte&   memByte();
    word&   memWord();
    dword&  memDword();

    Reg8&   reg8();
    Reg16&  reg16();
    Reg32&  reg32();

    dword   literal();
};

#endif