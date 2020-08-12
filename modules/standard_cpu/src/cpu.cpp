#include <dotcvm/dotcvm.hpp>
#include <modules/io_bus.hpp>
#include <modules/interrupt_bus.hpp>
#include <modules/memory.hpp>
#include <dotcvm/utils/log.hpp>

typedef uint32_t reg32;
typedef reg32 reg;
typedef uint16_t reg16;
typedef uint8_t  reg8;

struct gdt_reg
{
    reg32   address;
    reg16   offset;
};

static dotcvm_data s_dc;

static bool s_use_io_bus = true;
static io_bus*  sp_io_bus = nullptr;

static bool s_use_interrupt_bus = true;
static interrupt_bus* sp_interrupt_bus = nullptr;

static memory* s_mem;

enum arg_type { MEMORY8, MEMORY16, MEMORY32, LITERAL8, LITERAL16, LITERAL32, REG };

enum instruction { NO_OP  = 0xFF,
                   CPUINF = 0x00,
                   SET    = 0x10,
                   PUSH,
                   POP,
                   LGDTR,
                   SGDTR,
                   LDS,
                   LCS,
                   LSS,
                   SDS,
                   SCS,
                   SSS,
                   LIHP = 0x20,
                   SIHP,
                   INT,
                   IRET,
                   NOT = 0x30,
                   OR,
                   AND,
                   XOR,
                   NOR,
                   NAND,
                   XNOR,
                   ADD,
                   FADD,
                   SUB,
                   FSUB,
                   MUL,
                   FMUL,
                   DIV,
                   FDIV,
                   INTG = 0x70,
                   FLOAT,
                   JMP = 0x40,
                   CALL,
                   CMP,
                   FCMP,
                   UCMP,
                   JE, JZ, JG, JL, JEG, JEL,
                   RET,
                   HLT,
                   SLEEP,
                   LCPL = 0X60,
                   SCPL,
                   IN = 0X80,
                   OUT };

static uint16_t s_opcode = 0XFF00;
static instruction instr() { return (instruction)(s_opcode & 0x00FF); }
static uint8_t arg_descriptor() { return ((s_opcode >> 8) & 0x00FF); }

static bool s_interrupt = false;
static uint s_cycles = 0;

static arg_type arg0_type() { return (arg_type) ((arg_descriptor() >> 4  ) & 0x0F); }
static arg_type arg1_type() { return (arg_type) ( arg_descriptor() & 0x0F)        ; }

static uint arg0 = 0;
static uint arg1 = 0;

#pragma region REGISTERS
static reg ar = 0;
static reg br = 0;
static reg cr = 0;
static reg dr = 0;
static reg rr = 0; /* Result */
static reg fr = 0; /* Flags */
static reg tr = 0; /* Temporary */
static reg ur = 0; /* Temporary 2 */

static reg      spr    = 0; /* Stack pointer */
static reg      memr0  = 0; /* Memory address for arg0 */
static reg      memr1  = 0; /* Memory address for arg1 */
static gdt_reg  gdtr   = gdt_reg{.address = 0, .offset = 0}; /* Gdt register */
static reg16    dsr    = 0; /* data segment selector */
static reg16    csr    = 0; /* code segment selector */
static reg16    ssr    = 0; /* stack segment selector */

static reg irpr = 0; /* interrupt return pointer */
static reg ihpr = 0; /* interrupt handler pointer */

static reg rpr  = 0;  /* return pointer */
static reg ipr  = 0; /* instruction pointer */
static reg cr0  = 0; /* control 0, 1, 2 */
static reg cr1  = 0;
static reg cr2  = 0;

/* Free registers */
static reg far = 0;
static reg fbr = 0;
static reg fcr = 0;
static reg fdr = 0;
static reg frr = 0;

static reg8 cplr = 0; /* Current priviliege level */

static reg32 null_reg; // returned if reg id was not understood

#pragma endregion Definition of cpu registers

static reg32& get_reg(uint8_t id)
{
    switch (id)
    {
#define RET(n, r) case n: return r
    RET(0x00, ar);
    RET(0x01, br);
    RET(0x02, cr);
    RET(0x03, dr);
    RET(0x04, rr);
    RET(0x06, tr);
    RET(0x07, ur);
    RET(0x10, spr);
    RET(0x11, memr0);
    RET(0x12, memr1);
    RET(0x20, irpr);
    RET(0x30, rpr);
    RET(0x31, ipr);
    RET(0x50, far);
    RET(0x51, fbr);
    RET(0x52, fcr);
    RET(0x53, fdr);
    RET(0x54, frr);
#undef RET
    }
    WARN_M("Invalid reg id: " << id << " returning the null_reg");
    return null_reg;
}

static uint32_t s_interrupt_code = 0;

enum flag { Z = (1 << 0), // On if cmp arg0 = 0
            E = (1 << 1), // On if cmp arg0 = arg1
            G = (1 << 2), // on if cmp arg0 > arg1
            L = (1 << 3), // on if cmp arg0 < arg1
            I = (1 << 4)  // set by user with instruction cli abd sci // if set then interrupts are enabled
             };

bool s_on_interrupt = false;

bool flag_on(flag f)
{
    return (fr & (uint) f) != 0;
}

static uint bytes_to_uint(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
    return b3 << 24 | b2 << 16 | b1 << 8 | b0;
}

__export void module_report(uint additional_data0, uint additional_data1)
{
    if(additional_data0 == DC_CONNECTION_INAGREEMENT || additional_data0 == DC_CONNECTION_INEXISTANT)
    {
        if (additional_data1 == DC_STD_IOB)
        {
            WARN_M("CPU: Reported that io bus doesn't exist");
            s_use_io_bus = false;
        }
        else if(additional_data1 == DC_STD_ITB)
        {
            WARN_M("CPU: Reported that interrupt bus doesn't exist");
            s_use_interrupt_bus = false;
        }
    }
}

__export device_ptr module_create_device(dotcvm_data d)
{
    DEBUG_M("Creating cpu");
    s_dc = d;
    return nullptr;
}

__export void module_init()
{
    if(s_use_io_bus)
        sp_io_bus = (io_bus*) s_dc.fp_get_device(DC_STD_IOB);;
    if(s_use_interrupt_bus)
        sp_interrupt_bus = (interrupt_bus*) s_dc.fp_get_device(DC_STD_ITB);
    s_mem = (memory*) s_dc.fp_get_device(DC_STD_MEM);
    if(s_mem == nullptr)
        WARN_M("Memory is nullptr"); 
}

__export void module_clock(uint c)
{
    if(s_cycles == 0)
    {
        if(s_interrupt)
            s_interrupt = false;
        if((sp_interrupt_bus->mode == interrupt_mode::REQUEST && flag_on(flag::I) && !s_on_interrupt) || sp_interrupt_bus->mode == interrupt_mode::FORCE)
        {
            s_on_interrupt = true;
            s_interrupt = true;
            s_interrupt_code = sp_interrupt_bus->interrupt_code;
            s_cycles = 1;
        }
        if(!s_interrupt)
        {
            READ_MEM(ipr, WORD, s_mem);
            ipr += 2;
            s_cycles = -1; // Cycles -1 means fetch new instruction
        }
        return;
    }
    if(s_cycles == -1)
    {
        s_opcode = s_mem->data;
        s_cycles = 1;
        return;
    }
    if(s_interrupt)
    {
        static uint adr = 0;
        switch (s_cycles)
        {
#define SAVE4(n, v) case n: WRITE_MEM(adr, v, DWORD, s_mem); s_cycles++; adr -= 4; return
#define SAVE2(n, v) case n: WRITE_MEM(adr, v, WORD , s_mem); s_cycles++; adr -= 2; return
#define SAVE1(n, v) case n: WRITE_MEM(adr, v, BYTE , s_mem); s_cycles++; adr -= 1; return
        case 1: 
            WRITE_MEM(0x037FFFFF, ar, DWORD, s_mem);
            adr = 0x037FFFFB;
            return;
        SAVE4(2 , br  );
        SAVE4(3 , cr  );
        SAVE4(4 , dr  );
        SAVE4(5 , rr  );
        SAVE4(6 , fr  );
        SAVE4(7 , tr  );
        SAVE4(8 , ur  );
        SAVE4(9 , spr );
        SAVE2(10, dsr );
        SAVE2(11, csr );
        SAVE2(12, ssr );
        SAVE4(13, rpr );
        SAVE4(14, ipr );
        SAVE4(15, cr0 );
        SAVE4(16, cr1 );
        SAVE4(17, cr2 );
        SAVE4(18, far );
        SAVE4(19, fbr );
        SAVE4(20, fcr );
        SAVE4(21, fdr );
        SAVE4(22, frr );
        SAVE1(23, cplr);
        SAVE4(24, s_interrupt_code);
#undef SAVE4
#undef SAVE2
#undef SAVE1
        case 25:
            spr  = 0x037FFFA0;
            fr   = 0         ;
            dsr  = 0         ;
            csr  = 0         ;
            ssr  = 0         ;
            rpr  = 0x03000000;
            irpr = ipr       ;
            ipr  = ihpr      ;
            cplr = 0         ;
            s_cycles = 0;
            return;
        }
    }
    if(!s_interrupt && s_cycles != 0)
    {
        switch (instr())
        {
        case instruction::CPUINF:
            DEBUG_M("Instruction CPUINF, cycle: " << s_cycles);
            switch (s_cycles)
            {
            case 1:
                if(ar == 0) // cpu signature
                {
                    ar = bytes_to_uint('-', 'd', 't', 's');
                    br = bytes_to_uint('-', 'u', 'p', 'c');
                    cr = bytes_to_uint('-', 'd', 't', 's');
                    dr = bytes_to_uint('.', 'u', 'p', 'c');
                    s_cycles = 0;
                }
                return;
            }
        case instruction::SET:
            DEBUG_M("Intruction SET, cycle: " << s_cycles);
            switch (s_cycles)
            {
            case 1: // read arg0
                switch (arg0_type())
                {
                case arg_type::REG:
                    READ_MEM(ipr, BYTE, s_mem);
                    ipr++;
                    s_cycles = 2;
                    return;
                case arg_type::LITERAL8:
                    arg0 = ipr;
                    ipr += 1;
                    s_cycles = 3;
                    return;
                case arg_type::LITERAL16:
                    arg0 = ipr;
                    ipr += 2;
                    s_cycles = 3;
                    return;
                case arg_type::LITERAL32:
                    arg0 = ipr;
                    ipr += 4;
                    s_cycles = 3;
                    return;
                case arg_type::MEMORY8:
                case arg_type::MEMORY16:
                case arg_type::MEMORY32:
                    arg0 = memr0;
                    s_cycles = 3;
                    return;
                }
            case 2: // load register/mem
                arg0 = s_mem->data;
                s_cycles = 3;
                return;
            case 3: // read arg1
                switch (arg1_type())
                {
                case arg_type::REG:
                case arg_type::LITERAL8:
                    READ_MEM(ipr, BYTE, s_mem);
                    ipr += 1;
                    s_cycles = 4;
                    return;
                case arg_type::LITERAL16:
                    READ_MEM(ipr, WORD, s_mem);
                    ipr += 2;
                    s_cycles = 4;
                    return;
                case arg_type::LITERAL32:
                    READ_MEM(ipr, DWORD, s_mem);
                    ipr += 4;
                    s_cycles = 4;
                    return;
                case arg_type::MEMORY8:
                    READ_MEM(memr1, BYTE, s_mem);
                    s_cycles = 4;
                    return;
                case arg_type::MEMORY16:
                    READ_MEM(memr1, WORD, s_mem);
                    s_cycles = 4;
                    return;
                case arg_type::MEMORY32:
                    READ_MEM(memr1, DWORD, s_mem);
                    s_cycles = 4;
                    return;
                }
            case 4: // set $arg1 to arg1 value
                switch (arg1_type())
                {
                case arg_type::REG:
                    if(arg0_type() == arg_type::REG)
                    {
                        get_reg(arg0) = get_reg(arg1);
                        s_cycles = 0;
                        return;
                    }
                    arg1 = get_reg(s_mem->data);
                    s_cycles = 5;
                    return;
                case arg_type::LITERAL8:
                case arg_type::LITERAL16:
                case arg_type::LITERAL32:
                case arg_type::MEMORY8:
                case arg_type::MEMORY16:
                case arg_type::MEMORY32:
                    arg1 = s_mem->data;
                    s_cycles = 5;
                    return;
                }
            case 5: // set arg0 to $arg1
                switch (arg0_type())
                {
                case arg_type::REG:
                    get_reg(arg0) = arg1;
                    s_cycles = 0;
                    return;
                case arg_type::LITERAL8:
                case arg_type::MEMORY8:
                    WRITE_MEM(arg0, arg1, BYTE, s_mem);
                    s_cycles = 0;
                    return;
                case arg_type::LITERAL16:
                case arg_type::MEMORY16:
                    WRITE_MEM(arg0, arg1, WORD, s_mem);
                    s_cycles = 0;
                    return;
                case arg_type::LITERAL32:
                case arg_type::MEMORY32:
                    WRITE_MEM(arg0, arg1, DWORD, s_mem);
                    s_cycles = 0;
                    return;
                }
            }
        case instruction::NO_OP:
            DEBUG_M("No operetion");
            s_cycles = 0;
            return;   
        default:
            WARN_M("Unknow instruction: " << instr());
            ipr++; // TODO: fire an interrupt here
            s_cycles = 0;
            break;
        }
    }
}

__export void module_destroy_device(device_ptr i)
{
#ifdef DEBUG
    LOG_M("Cpu state: ");
    LOG_M("\ta: " << ar << ",\tb: " << br);
    LOG_M("\tc: " << cr << ",\td: " << dr);
#endif
}
