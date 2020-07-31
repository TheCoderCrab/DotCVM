#ifndef CPU_INTERNAL_H
#define CPU_INTERNAL_H

#include <DotCVM/devices/reg_internal.h>
#include <DotCVM/devices/instructions.h>
#include <stdint.h>

class CPU
{
public:
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
    Reg32 memr0; // Holds the address to be read from memory for arg0
    Reg32 memr1; // Holds the address to be read from memory for arg1

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

    // NULL registers: returned when given an invalid id
    Reg8    nullreg8;
    Reg16   nullreg16;
    Reg32   nullreg32;

    // Devices
    Memory*         m_Mem;
    Drive*          m_Drive;
    ScreenDevice*   m_Scr;
    Keyboard*       m_Keyboard;
    Mouse*          m_Mouse;
    DebugConsole*   m_DebugConsole;

    CPU(uint32_t memSize, uint32_t sriveSizeIfNotExist);
    ~CPU();

    void update();

    Memory&         mem();
    Drive&          drive();
    ScreenDevice&   screen();
    Keyboard&       keyboard();
    Mouse&          mouse();
    DebugConsole&   debugConsole();

    byte            instruction(); // Returns the instruction at ip
    Argument        arg     (byte argPosition); // Returns argument data of the argument at (ip + 1) position <argPosition>
    ArgumentType    argType (byte argPosition); // Returns argument type of the argument at (ip + 1) position <argPosition>
    Reg8&           reg8    (byte id         );
    Reg16&          reg16   (byte id         );
    Reg32&          reg32   (byte id         );
    byte            regSize (byte id         );

    bool            execute(); // Executes the instruction pointed to by register ip
                               // Returns wether or not to move ip to the next instruction
    
    // Instructions
    // Returns wether or not register ip should be set to next instruction,
    // true = change reg ip, false = don't

    // Misc: 0XXXH

    // cpuinf: 00XXH
    bool cpuinf() ;

    // Data management: 1XXXH
    // set: 10XXH
    bool set(Argument dest, Argument val);
    // push: 11XXH
    bool push(Argument val);
    // pop: 12XXH
    bool pop(Argument val);
    // lgdtr: 13XXH
    bool lgdtr(Argument adr);
    // sgdtr: 14XXH
    bool sgdtr(Argument adr);
    // lds: 15XXH
    bool lds(Argument val);
    // lcs: 16XXH
    bool lcs(Argument val);
    // lss: 17XXH
    bool lss(Argument val);
    // sds: 18XXH
    bool sds(Argument ctnr);
    // scs: 19XXH
    bool scs(Argument ctnr);
    // sss: 18XXH
    bool sss(Argument ctnr);

    //Interrupts: 2XXXH
    // lihp: 21XXH
    bool lihp(Argument val);
    // sihp: 22XXH
    bool sihp(Argument ctnr);
    // int: 23XXH
    bool intr(Argument intCode, bool hardwareInterrupt);
    // iret: 24XXH
    bool iret();

    // Arithmetic and logic: 3XXXH
    // not: 30XXH
    bool not_(Argument result, Argument operand);
    bool not_(Argument operand);
    // add: 31XXH
    bool add(Argument op1, Argument op2);
    // sub: 32XXH
    bool sub(Argument op1, Argument op2);
    // mul: 33XXH
    bool mul(Argument op1, Argument op2);
    // div: 34XXH
    bool div(Argument op1, Argument op2);
    // fadd: 31XXH
    bool fadd(Argument op1, Argument op2);
    // fsub: 32XXH
    bool fsub(Argument op1, Argument op2);
    // fmul: 33XXH
    bool fmul(Argument op1, Argument op2);
    // fdiv: 34XXH
    bool fdiv(Argument op1, Argument op2);

    // Execution flow: 4XXXH
    // jmp: 40XXH
    bool jmp(Argument adr);
    // call: 41XXH
    bool call(Argument adr);
    // cmp: 42XXH | fcmp: 43XXH | ucmp: 44XXH
    bool cmp(Argument op1, Argument op2);
    // je: 46XXH | jz: 47XXH | jg: 48XXH | jl: 49XXH | jeg: 4AXXH | jel: 4BXXH | jiof: 4CXXH
    bool joc(Argument adr, bool c); // Jump on condition
    // ret: 4DXXH
    bool ret();
    // hlt: 4EXXH
    bool hlt();
    // sleep: 4FXXH
    bool sleep(Argument time);
    // Control: 5XXXH
    //Not defined yet

    // Security: 6XXXH
    // lcpl: 60XXH
    bool lcpl(Argument ncpl);
    // scpl: 61XXH
    bool scpl(Argument ctn);
    // Arithmetic and logic 2: 7XXH
    bool intg(Argument op1, Argument op2);
    bool ifloat(Argument op1, Argument op2);
    // IO: 8XXXH
    // out: 80XXH
    bool out(Argument adr, Argument code);
    // in: 81XXH
    bool in(Argument adr, Argument ret);
};

#endif