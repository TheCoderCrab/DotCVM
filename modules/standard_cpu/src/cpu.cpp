#include <dotcvm/dotcvm.hpp>
#include <dotcvm/utils/debug.hpp>
#include <modules/io_bus.hpp>
#include <modules/interrupt_bus.hpp>
#include <modules/memory.hpp>
#include <dotcvm/utils/log.hpp>
#include <locale>
#include <string>
#include <bits/stdc++.h>

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

enum arg_type { DISABLED, MEMORY8, MEMORY16, MEMORY32, LITERAL8, LITERAL16, LITERAL32, REG };

enum instruction { NO_OP  = 0xFF,
                   CPUINF = 0x00,
                   SET    = 0x10,
                   PUSH4,
                   PUSH2,
                   PUSH1,
                   POP4,
                   POP2,
                   POP1,
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

static uint s_cycles = 0;
static bool s_interrupt_setup           = false ; // Setting up an interrupt
static bool s_on_interrupt              = false ; // iret not called yet
static bool s_software_interrupt_req    = false ; // set by 'int'
static uint s_software_interrupt_code   = 0     ;
static uint s_interrupt_code            = 0     ;

static arg_type arg0_type() { return (arg_type) ((arg_descriptor() >> 4  ) & 0x0F); }
static arg_type arg1_type() { return (arg_type) ( arg_descriptor() & 0x0F)        ; }

static uint arg0   = 0;
static uint arg0_a = 0;
static uint arg1   = 0;
static uint arg1_a = 0; 

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

#pragma endregion

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
enum flag { Z = (1 << 0), // On if cmp arg0 = 0
            E = (1 << 1), // On if cmp arg0 = arg1
            G = (1 << 2), // on if cmp arg0 > arg1
            L = (1 << 3), // on if cmp arg0 < arg1
            I = (1 << 4)  // set by user with instruction cli abd sci // if set then interrupts are enabled
             };
bool flag_on(flag f)
{
    return (fr & (uint) f) != 0;
}
static uint bytes_to_uint(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
    return b3 << 24 | b2 << 16 | b1 << 8 | b0;
}
/// Returns code address with segment offset
static uint c_adr(uint adr)
{
    return adr; // TODO: implement c_adr
}
/// Returns data address with segment offset
static uint d_adr(uint adr)
{
    return adr; // TODO: implement d_adr
}
/// Returns stack address with stack segment offset
static uint s_adr(uint adr)
{
    return adr; // TODO: implement s_adr
}
static void load_arg(uint& arg_a, arg_type type)
{
    switch(type)
    {
    case arg_type::DISABLED:
        arg_a = 0;
        return;
    case arg_type::REG:
    case arg_type::LITERAL8:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg_a = ipr; // for register arg_a should be set manually on the next clock
        ipr += 1;
        return;
    case arg_type::LITERAL16:
        READ_MEM(c_adr(ipr), WORD, s_mem);
        arg_a = ipr;
        ipr += 2;
        return;
    case arg_type::LITERAL32:
        READ_MEM(c_adr(ipr), DWORD, s_mem);
        arg_a = ipr;
        ipr += 4;
        return;
    case arg_type::MEMORY8:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg_a = memr0;
        return;
    case arg_type::MEMORY16:
        READ_MEM(d_adr(memr0), WORD, s_mem);
        arg_a = memr0;
        return;
    case arg_type::MEMORY32:
        READ_MEM(d_adr(memr0), DWORD, s_mem);
        arg_a = memr0;
        return;
    }
}
static void store_arg(uint arg_a, arg_type type, uint v)
{
    switch(type)
    {
    case arg_type::REG:
        get_reg(arg_a) = v;
        return;
    case arg_type::LITERAL8:
    case arg_type::MEMORY8:
        WRITE_MEM(arg_a, v, BYTE, s_mem);
        return;
    case arg_type::LITERAL16:
    case arg_type::MEMORY16:
        WRITE_MEM(arg_a, v, WORD, s_mem);
        return;
    case arg_type::LITERAL32:
    case arg_type::MEMORY32:
        WRITE_MEM(arg_a, v, DWORD, s_mem);
        return;
    }
}

static bool check_cpl(uint min)
{
    if(cplr > min)
    {
        // TODO: fire an interrupt here
        s_cycles = 0;
        return false;
    }
    return true;
}
#define NCPL(n) if(!check_cpl(n)) return;

#pragma region INSTRUCTIONS

static void i_cpuinf()
{
    if(arg0 == 0) // CPU vendor
    {
        ar = bytes_to_uint('n', 'n', 'n', 'n');
        br = bytes_to_uint('n', 'n', 'n', 'n');
        cr = bytes_to_uint('n', 'n', 'n', 'n');
        dr = bytes_to_uint('n', 'n', 'n', 'n');
        s_cycles = 0;
    }
}
static void i_set()
{
    store_arg(arg0_a, arg0_type(), arg1);
    s_cycles = 0;
}
template<uint s> static void i_push()
{
    WRITE_MEM_E(s_adr(spr), arg0, (memory_mode) s, s_mem);
    spr += s;
    s_cycles = 0;
}
template<uint s> static void i_pop()
{
    switch(s_cycles)
    {
    case 1:
        spr -= s;
        READ_MEM_E(s_adr(spr), (memory_mode) s, s_mem);
        s_cycles = 2;
        return;
    case 2:
        store_arg(arg0_a, arg0_type(), s_mem->data);
        s_cycles = 0;
        return;
    }
}
static void i_lgdtr()
{
    NCPL(0)
    switch(s_cycles)
    {
    case 1:
        READ_MEM(d_adr(arg0), WORD, s_mem);
        s_cycles = 2;
        return;
    case 2:
        arg1 = s_mem->data; // FIXME: maybe not use arg1 as temporary variable
        READ_MEM(d_adr(arg0 + 2), DWORD, s_mem);
        s_cycles = 3;
        return;
    case 3:
        gdtr.offset  = arg1;
        gdtr.address = s_mem->data;
        s_cycles = 0;
        return;
    }
}
static void i_sgdtr()
{
    switch(s_cycles)
    {
    case 1:
        WRITE_MEM(d_adr(arg0), gdtr.offset, WORD, s_mem);
        s_cycles = 2;
        return;
    case 2:
        WRITE_MEM(d_adr(arg0 + 2), gdtr.address, DWORD, s_mem);
        s_cycles = 0;
        return;
    }
}
template<reg16& reg> static void i_lseg()
{
    reg = arg0;
    s_cycles = 0;
}
template<reg16& reg> static void i_sseg()
{
   store_arg(arg0_a, arg0_type(), reg);
   s_cycles = 0;
}


static void i_hlt()
{
    s_dc.fp_shutdown(0, "CPU halted");
    s_cycles = 0;
    return;
}

#pragma endregion

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
    if(s_interrupt_setup)
    {
        static uint adr = 0;
        switch (s_cycles)
        {
#define SAVE4(n, v) case n: WRITE_MEM(adr, v, DWORD, s_mem); s_cycles++; adr -= 4; return
#define SAVE2(n, v) case n: WRITE_MEM(adr, v, WORD , s_mem); s_cycles++; adr -= 2; return
#define SAVE1(n, v) case n: WRITE_MEM(adr, v, BYTE , s_mem); s_cycles++; adr -= 1; return
        case 0: 
            WRITE_MEM(0x037FFFFF, ar, DWORD, s_mem);
            adr = 0x037FFFFB;
            return;
        SAVE4(1 , br  );
        SAVE4(2 , cr  );
        SAVE4(3 , dr  );
        SAVE4(4 , rr  );
        SAVE4(5 , fr  );
        SAVE4(6 , tr  );
        SAVE4(7 , ur  );
        SAVE4(8 , spr );
        SAVE2(9, dsr );
        SAVE2(10, csr );
        SAVE2(11, ssr );
        SAVE4(12, rpr );
        SAVE4(13, ipr );
        SAVE4(14, cr0 );
        SAVE4(15, cr1 );
        SAVE4(16, cr2 );
        SAVE4(17, far );
        SAVE4(18, fbr );
        SAVE4(19, fcr );
        SAVE4(20, fdr );
        SAVE4(21, frr );
        SAVE1(22, cplr);
        SAVE4(23, s_interrupt_code);
#undef SAVE4
#undef SAVE2
#undef SAVE1
        case 24:
            spr  = 0x037FFFA0           ;
            dsr  = 0                    ;
            csr  = 0                    ;
            ssr  = 0                    ;
            rpr  = 0x03000000           ;
            irpr = ipr                  ;
            ipr  = ihpr                 ;
            cplr = 0                    ;
            s_cycles = 0                ;
            s_interrupt_setup = false   ;
            s_on_interrupt    = true    ;
            return;
        }
    }
    else
    {
        switch(s_cycles)
        {
#pragma region INSTRUCTION_SETUP
        case 0: // otherwise fetch new instruction
            if(!s_on_interrupt) // Check if there is an interrupt waiting
            {
                if((sp_interrupt_bus->mode == interrupt_mode::REQUEST && flag_on(flag::I)) || sp_interrupt_bus->mode == interrupt_mode::FORCE)
                {
                    s_interrupt_setup       = true                              ;
                    s_interrupt_code        = sp_interrupt_bus->interrupt_code  ;
                    sp_interrupt_bus->mode  = interrupt_mode::NONE              ;
                    return;
                }
                else if(s_software_interrupt_req)
                {
                    s_software_interrupt_req     = false                    ;
                    s_interrupt_setup            = true                     ;
                    s_interrupt_code             = s_software_interrupt_code;
                    return;
                }
            }
            // If we are here then there is no interrupt to setup
            // Fetch new instruction
            READ_MEM(c_adr(ipr), WORD, s_mem);
            ipr += 2;
            s_cycles = -1;
            return;
        case (uint) -1: // continue fetching instruction
            s_opcode = s_mem->data; // Read from cycle 0
            load_arg(arg0_a, arg0_type());
            s_cycles = -2;
            return;
        case (uint) -2:
            if(arg0_type() == arg_type::REG)
            {
                arg0_a = s_mem->data;
                arg0   = get_reg(arg0_a);
            }
            else
                arg0 = s_mem->data;
            load_arg(arg1_a, arg1_type());
            s_cycles = -3;
            return;
        case (uint) - 3:
            if(arg1_type() == arg_type::REG)
            {
                arg1_a = s_mem->data;
                arg1   = get_reg(arg1_a);
            }
            else
                arg1 = s_mem->data;
            // instruction turn
            s_cycles = 1;
            return;
#pragma endregion
        default: // the instruction should take care of it
            switch (instr())
            {
#define INSTRUCTION(i, f) case instruction::i: DEBUG_M("Instruction: " << #i << ", cycle: " << s_cycles); f(); return
                INSTRUCTION(CPUINF  , i_cpuinf      );
                INSTRUCTION(SET     , i_set         );
                INSTRUCTION(PUSH4   , i_push<4>     );
                INSTRUCTION(PUSH2   , i_push<2>     );
                INSTRUCTION(PUSH1   , i_push<1>     );
                INSTRUCTION(POP4    , i_pop<4>      );
                INSTRUCTION(POP2    , i_pop<2>      );
                INSTRUCTION(POP1    , i_pop<1>      );
                INSTRUCTION(LGDTR   , i_lgdtr       );
                INSTRUCTION(SGDTR   , i_sgdtr       );
                INSTRUCTION(LDS     , i_lseg<dsr>   );
                INSTRUCTION(LCS     , i_lseg<csr>   );
                INSTRUCTION(LSS     , i_lseg<ssr>   );
                INSTRUCTION(SDS     , i_sseg<dsr>   );
                INSTRUCTION(SCS     , i_sseg<csr>   );
                INSTRUCTION(SSS     , i_sseg<ssr>   );


                INSTRUCTION(HLT     , i_hlt     );
#undef INSTRUCTION
            }
        }
    }

}
__export void module_destroy_device(device_ptr i)
{
    LOG_M("Cpu state: ");
    LOG_M("\ta      : " << ar   << "\tb : " << br);
    LOG_M("\tc      : " << cr   << "\td : " << dr);
    LOG_M("\tds     : " << dsr  << "\tcs: " << csr << "\tss: " << ssr);
    LOG_M("\tsp     : " << spr  << "\tmemr0: " << memr0 << "\tmemr1: " << memr1);
    LOG_M("\tihp    : " << ihpr << "\tirp: " << irpr);
    LOG_M("\tip     : " << ipr  << "\trp: " << rpr);
    LOG_M("\tGDTR   : offset: " << gdtr.offset << ", base: " << gdtr.address);
    LOG_M("\tt      : " << tr   << "\tu : " << ur);
}
