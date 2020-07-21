#ifndef CPU_H
#define CPU_H


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

#endif