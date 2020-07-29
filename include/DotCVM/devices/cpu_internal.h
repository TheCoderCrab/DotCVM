#ifndef CPU_H
#define CPU_H

#include <DotCVM/devices/reg_internal.h>
#include <DotCVM/devices/instructions.h>

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
    Reg32  memr; // Holds the address to be read from memory


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
    Memory*         m_Mem;
    Drive*          m_Drive;
    ScreenDevice*   m_Scr;
    Keyboard*       m_Keyboard;
    Mouse*          m_Mouse;
    DebugConsole*   m_DebugConsole;

public:
    CPU(uint32_t memSize, uint32_t sriveSizeIfNotExist);
    ~CPU();

    void update();

    Memory&         mem();
    Drive&          drive();
    ScreenDevice&   screen();
    Keyboard&       keyboard();
    Mouse&          mouse();
    DebugConsole&   debugConsole();

    Argument        arg(byte argPosition); // Returns argument data of the argument at (ip + 1) position <argPosition>
    ArgumentType    argType(byte argPosition); // Returns argument type of the argument at (ip + 1) position <argPosition>
    

    
    // Instructions
    // Returns wether or not register ip should be set to next instruction,
    // true = change reg ip, false = don't

    // Misc: 0XXXH

    // cpuinf: 00XXH
    bool cpuinf() ;

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
    bool lgdtr(address adr);
    // sgdtr: 14XXH
    bool sgdtr(address adr);
    // lds: 15XXH
    bool lds(word val);
    // lcs: 16XXH
    bool lcs(word val);
    // lss: 17XXH
    bool lss(word val);
    // sds: 18XXH
    bool sds(word& ctnr);
    // scs: 19XXH
    bool scs(word& ctnr);
    // sss: 18XXH
    bool sss(word& ctnr);

    //Interrupts: 2XXXH
    // lihp: 21XXH
    bool lihp(dword val);
    // sihp: 22XXH
    bool sihp(dword& ctnr);
    // int: 23XXH
    bool intr(byte intCode, bool hardwareInterrupt);
    // iret: 24XXH
    bool iret();

    // Arithmetic and logic: 3XXXH
    // not: 30XXH
    bool not_(dword& result, dword operand);
    bool not_(dword& operand);
    // add: 31XXH
    bool add(dword& op1, dword op2);
    // sub: 32XXH
    bool sub(dword& op1, dword op2);
    // mul: 33XXH
    bool mul(dword& op1, dword op2);
    // div: 34XXH
    bool div(dword& op1, dword op2);
    // fadd: 31XXH
    bool fadd(float& op1, float op2);
    // fsub: 32XXH
    bool fsub(float& op1, float op2);
    // fmul: 33XXH
    bool fmul(float& op1, float op2);
    // fdiv: 34XXH
    bool fdiv(float& op1, float op2);

    // Execution flow: 4XXXH
    // jmp: 40XXH
    bool jmp(address adr);
    // call: 41XXH
    bool call(address adr);
    // cmp: 42XXH | fcmp: 43XXH | ucmp: 44XXH
    template<typename T> bool cmp(T op1, T op2)
    {
        f(0, op1 > op2);
        f(1, a < b);
        return true;
    }
    // je: 46XXH | jz: 47XXH | jg: 48XXH | jl: 49XXH | jeg: 4AXXH | jel: 4BXXH | jiof: 4CXXH
    bool joc(address adr, bool c); // Jump on condition
    // ret: 4DXXH
    bool ret();
    // hlt: 4EXXH
    bool hlt();
    // sleep: 4FXXH
    bool sleep(uint32_t time);
    // Control: 5XXXH
    //Not defined yet

    // Security: 6XXXH
    // lcpl: 60XXH
    bool lcpl(byte ncpl);
    // scpl: 61XXH
    bool scpl(byte& ctn);
    // Arithmetic and logic 2: 7XXH
    bool intg(dword& op1, float op2);
    bool ifloat(float& op1, dword op2);
    // IO: 8XXXH
    // out: 80XXH
    bool out(dword adr, dword code);
    // in: 81XXH
    bool in(dword adr, dword& ret);
};

#endif