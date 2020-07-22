#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/utils.h>
#include <DotCVM/utils/err_code.h>
#include <DotCVM/utils/log.h>
#include <DotCVM/gui/window.h>

CPU::CPU(uint32_t memSize, uint32_t driveSizeIfNotExist)
{
    debug("Creating CPU");
    m_Mem            = new Memory(memSize);
    m_Drive          = new Drive(driveSizeIfNotExist);
    m_Scr            = new ScreenDevice(m_Mem);
    m_Keyboard       = new Keyboard();
    m_Mouse          = new Mouse();
    m_DebugConsole   = new DebugConsole();
    debug("CPU created");
}
CPU::~CPU()
{
    delete m_Mem;
    delete m_Drive;
    delete m_Scr;
    delete m_Keyboard;
    delete m_Mouse;
    delete m_DebugConsole;
}

void CPU::update()
{
    m_Scr->update();
}
Memory&       CPU::mem()          { return *m_Mem;            }
Drive&        CPU::drive()        { return *m_Drive;          }
ScreenDevice& CPU::screen()       { return *m_Scr;            }
Keyboard&     CPU::keyboard()     { return *m_Keyboard;       }
Mouse&        CPU::mouse()        { return *m_Mouse;          }
DebugConsole& CPU::debugConsole() { return *m_DebugConsole;   }

bool CPU::cpuinf() // 00XXH
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



bool CPU::lgdtr(address adr)
{
    gdtr.load(m_Mem, adr);
    return true;
}

bool CPU::sgdtr(address adr)
{
    gdtr.store(m_Mem, adr);
    return true;
}

bool CPU::lds(word val)
{
    ds = val;
    return true;
}

bool CPU::lcs(word val)
{
    cs = val;
    return true;
}

bool CPU::lss(word val)
{
    ss = val;
    return true;
}

bool CPU::sds(word& ctnr)
{
    ctnr = ds;
    return true;
}


bool CPU::scs(word& ctnr)
{
    ctnr = cs;
    return true;
}


bool CPU::sss(word& ctnr)
{
    ctnr = ss;
    return true;
}



bool CPU::lihp(dword val)
{
    ihp = val;
    return true;
}


bool CPU::sihp(dword& ctnr)
{
    ctnr = ihp;
    return true;
}


bool CPU::intr(byte intCode)
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


bool CPU::iret()
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



bool CPU::not_(dword& result, dword operand)
{
    result = !(operand);
    return true;
}
bool CPU::not_(dword& operand)
{
    operand = !operand;
    return true;
}


bool CPU::add(dword& op1, dword op2)
{
    op1 += op2;
    return true;
}


bool CPU::sub(dword& op1, dword op2)
{
    op1 -= op2;
    return true;
}


bool CPU::mul(dword& op1, dword op2)
{
    op1 *= op2;
    return true;
}


bool CPU::div(dword& op1, dword op2)
{
    op1 /= op2;
    return true;
}


bool CPU::fadd(float& op1, float op2)
{
    op1 += op2;
    return true;
}


bool CPU::fsub(float& op1, float op2)
{
    op1 -= op2;
    return true;
}


bool CPU::fmul(float& op1, float op2)
{
    op1 *= op2;
    return true;
}


bool CPU::fdiv(float& op1, float op2)
{
    op1 /= op2;
    return true;
}



bool CPU::jmp(address adr)
{
    ip = adr;
    return false;
}


bool CPU::call(address adr)
{
    rp = ip; // ALERT: Make so that it returns to the next instruction
    ip = adr;
    return false;
}


bool CPU::joc(address adr, bool c)
{
    if(c)
        ip = adr;
    return false;
}


bool CPU::ret()
{
    ip = rp;
    return false;
}


bool CPU::hlt()
{
    return false; // by returning false ip won't change so we hang here.
}


bool CPU::sleep()
{
    // TODO: implemnt sleep
    return true;
}


bool CPU::lcpl(byte ncpl)
{
    // TODO: check for 0 <= cpl <= 3
    cpl = ncpl;
    return true;
}


bool CPU::scpl(byte& ctn)
{
    ctn = cpl;
    return true;
}


bool CPU::intg(dword& op1, float op2)
{
    op1 = (dword) op2;
    return true;
}
bool CPU::ifloat(float& op1, dword op2)
{
    op1 = (float) op2;
    return true;
}

bool CPU::out(dword adr, dword code)
{
    switch(adr)
    {
        case IO_DRIVE_ADR:
            m_Drive->in(m_Mem, code);
            break;
        case IO_SCR_ADR:
            m_Scr->in(code);
            break;
        case IO_DEBUG_CONSOLE_ADR:
            m_DebugConsole->in(code);
            break;
        default:
            requestClose(IO_INVALID_OPERATION, "Invalid IO address");
            break;
    }
    return true;
}


bool CPU::in(dword adr, dword& ret)
{
    switch (adr)
    {
    case IO_DRIVE_ADR:
        ret = m_Drive->out();
        break;
    case IO_MEM_ADR:
        ret = m_Mem->out();
        break;
    case IO_KEYBOARD_ADR:
        ret = m_Keyboard->out();
    case IO_MOUSE_ADR:
        ret = m_Mouse->out();
    default:
        requestClose(IO_INVALID_OPERATION, "Invalid IO address");
        break;
    }
    return true;
}
