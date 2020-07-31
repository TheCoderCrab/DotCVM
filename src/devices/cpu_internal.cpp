#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/utils.h>
#include <DotCVM/utils/err_code.h>
#include <DotCVM/utils/log.h>
#include <DotCVM/gui/window.h>
#include <DotCVM/devices/instructions.h>
#include <unistd.h>
#include <DotCVM/devices/interrupts.h>
#include <DotCVM/devices/reg.h>

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
    if(execute())
        ip += 10;
    m_Scr->update();
}
Memory&       CPU::mem()          { return *m_Mem;            }
Drive&        CPU::drive()        { return *m_Drive;          }
ScreenDevice& CPU::screen()       { return *m_Scr;            }
Keyboard&     CPU::keyboard()     { return *m_Keyboard;       }
Mouse&        CPU::mouse()        { return *m_Mouse;          }
DebugConsole& CPU::debugConsole() { return *m_DebugConsole;   }

byte          CPU::instruction()
{
    return mem().at<byte>(ip);
}

Argument CPU::arg(byte argPosition)
{
    if(argPosition == 0)
        return Argument(this, argType(argPosition), mem().at<dword>(ip + 2), 0);
    return Argument(this, argType(argPosition), mem().at<dword>(ip + 6), 1);
}

ArgumentType CPU::argType(byte argPosition)
{
    if(argPosition == 0)
        return (ArgumentType)(mem().at<byte>(ip + 1) & 0x0F);
    return (ArgumentType)((mem().at<byte>(ip + 1) & 0xF0) >> 4);
}

Reg8&   CPU::reg8(byte id)
{
    err("Trying to get an inaccessible register8"); // Since all 8-bit registers are inaccessible
    if(id == REG_I_CPL)
        return cpl;
    err("No 8-bit register with id: " << id << " returning nullreg8");
    return nullreg8;
}
Reg16&  CPU::reg16(byte id)
{
    err("Trying to get an inaccessible register16"); // Since all 16-bit registers are inaccessible
    switch (id)
    {
    case REG_I_CS: return cs;
    case REG_I_DS: return ds;
    case REG_I_SS: return ss;
    default:
        err("No 16-bit register with id: " << id << " return nullreg16");
        return nullreg16;
    }
}
Reg32&  CPU::reg32(byte id)
{
    switch(id)
    {
        case REG_A: return a;
        case REG_B: return b;
        case REG_C: return c;
        case REG_D: return d;
        case REG_R: return r;
        case REG_F: return f;
        case REG_T: return t;
        case REG_U: return u;
        case REG_SP: return sp;
        case REG_IRP: return irp;
        case REG_IHP: return ihp;
        case REG_RP: return rp;
        case REG_IP: return ip;
        case REG_FA: return fa;
        case REG_FB: return fb;
        case REG_FC: return fc;
        case REG_FD: return fd;
        case REG_FR: return fr;
        case REG_I_MEMR0:
            err("Trying to access inaccessible register memr0");
            return memr0;
        case REG_I_MEMR1:
            err("Trying to access inaccessible register memr1");
            return memr1;
        case REG_I_CR0:
            err("Trying to access inaccessible register cr0");
            return cr0;
        case REG_I_CR1:
            err("Trying to access inaccessible register cr1");
            return cr1;
        case REG_I_CR2:
            err("Trying to access inaccessible register cr2");
            return cr2;
        default:
            err("Invalid register id: " << id << " returning nullreg32");
            return nullreg32;
    }
}

byte CPU::regSize(byte id)
{
    switch (id)
    {
    case REG_I_CPL:
        return 8;
    case REG_I_CS:
    case REG_I_DS:
    case REG_I_SS:
        return 16;
    case REG_I_GDTR: return 48;
    default: return 32; // TODO: The id may be invalid
    }
}

bool CPU::execute()
{
    switch (instruction())
    {
    case INSTRUCTION_SET:
        if(ARG_WRITABLE(argType(0)))
            return set()
    default:
        break;
    }
    return false;
}


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



bool CPU::lgdtr(Argument adr)
{
    gdtr.load(m_Mem, adr);
    return true;
}

bool CPU::sgdtr(Argument adr)
{
    gdtr.store(m_Mem, adr);
    return true;
}

bool CPU::lds(Argument val)
{
    ds = val;
    return true;
}

bool CPU::lcs(Argument val)
{
    cs = val;
    return true;
}

bool CPU::lss(Argument val)
{
    ss = val;
    return true;
}

bool CPU::sds(Argument ctnr)
{
    ctnr = ds;
    return true;
}


bool CPU::scs(Argument ctnr)
{
    ctnr = cs;
    return true;
}


bool CPU::sss(Argument ctnr)
{
    ctnr = ss;
    return true;
}



bool CPU::lihp(Argument val)
{
    ihp = val;
    return true;
}


bool CPU::sihp(Argument ctnr)
{
    ctnr = ihp;
    return true;
}


bool CPU::intr(Argument intCode, bool hardwareInterrupt)
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

    irp = hardwareInterrupt ? ip.uval32 : ip + 10;

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



bool CPU::not_(Argument result, Argument operand)
{
    result = !(operand);
    return true;
}
bool CPU::not_(Argument operand)
{
    operand = !operand;
    return true;
}


bool CPU::add(Argument op1, Argument op2)
{
    op1 += op2;
    return true;
}


bool CPU::sub(Argument op1, Argument op2)
{
    op1 -= op2;
    return true;
}


bool CPU::mul(Argument op1, Argument op2)
{
    op1 *= op2;
    return true;
}


bool CPU::div(Argument op1, Argument op2)
{
    op1 /= op2;
    return true;
}


bool CPU::fadd(Argument op1, Argument op2)
{
    op1 += op2;
    return true;
}


bool CPU::fsub(Argument op1, Argument op2)
{
    op1 -= op2;
    return true;
}


bool CPU::fmul(Argument op1, Argument op2)
{
    op1 *= op2;
    return true;
}


bool CPU::fdiv(Argument op1, Argument op2)
{
    op1 /= op2;
    return true;
}



bool CPU::jmp(Argument adr)
{
    ip = adr;
    return false;
}


bool CPU::call(Argument adr)
{
    rp = ip + 10;
    ip = adr;
    return false;
}


bool CPU::joc(Argument adr, bool c)
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


bool CPU::sleep(Argument time)
{
    usleep(time);
    return true;
}


bool CPU::lcpl(Argument ncpl)
{
    // TODO: check for 0 <= cpl <= 3
    cpl = ncpl;
    return true;
}


bool CPU::scpl(Argument ctn)
{
    ctn = cpl;
    return true;
}


bool CPU::intg(Argument op1, Argument op2)
{
    op1 = (dword) op2;
    return true;
}
bool CPU::ifloat(Argument op1, Argument op2)
{
    op1 = op2.asFloat();
    return true;
}

bool CPU::out(Argument adr, Argument code)
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


bool CPU::in(Argument adr, Argument ret)
{
    switch(adr)
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
