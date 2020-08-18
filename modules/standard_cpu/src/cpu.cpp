#include <dotcvm/dotcvm.hpp>
#include <dotcvm/utils/debug.hpp>
#include <modules/io_bus.hpp>
#include <modules/interrupt_bus.hpp>
#include <modules/memory.hpp>
#include <dotcvm/utils/log.hpp>
#include <locale>
#include <string>
#include <chrono>
#include <thread>
#include <bits/stdc++.h>

#define DEFAULT_FLAGS (uint) flag::I

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

enum flag { Z = (1 << 0), // On if cmp arg0 = 0
            E = (1 << 1), // On if cmp arg0 = arg1
            G = (1 << 2), // on if cmp arg0 > arg1
            L = (1 << 3), // on if cmp arg0 < arg1
            I = (1 << 4)  // set by user with instruction cli abd sci // if set then interrupts are enabled
             };

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
                   SL = 0x70,
                   SR = 0x71,
                   INTG,
                   FLOAT,
                   JMP = 0x40,
                   CALL,
                   CMP,
                   FCMP,
                   UCMP,
                   JE, JZ, JG, JL, JEG, JEL,
                   RET,
                   HLT,
                   SLEEPS,
                   SLEEPMS,
                   SLEEPMCS,
                   LCPL = 0X60,
                   SCPL,
                   IN = 0X80,
                   OUT };

static uint16_t s_opcode = 0XFF00;
static instruction instr() { return (instruction)(s_opcode & 0x00FF); }
static uint8_t arg_descriptor() { return ((s_opcode >> 8) & 0x00FF); }

enum sleep_mode { S, MS, MCS };
static sleep_mode s_sleep_mode;
static std::thread s_sleep_thread;
static bool s_sleep  = false;
static uint s_sleep_time = 0;
static uint s_cycles = 0    ;
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
static reg fr = DEFAULT_FLAGS; /* Flags */
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
bool flag_on(flag f)
{
    return (fr & (uint) f) != 0;
}
void set_flag(flag f)
{
    fr = fr | (uint) f;
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

static uint get_user_defined_needed_cpl()
{
    return 3; // TODO: implement user defined cpl
}

static union
{
    uint u;
    float f;
} u_f;
static float uint_as_float(uint i)
{
    u_f.u = i;
    return u_f.f;
}
static uint float_as_uint(float f)
{
    u_f.f = f;
    return u_f.u;
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
#define UCPL    NCPL(get_user_defined_needed_cpl())

#pragma region INSTRUCTIONS

static void i_cpuinf()
{
    if(arg0 == 0) // CPU vendor
    {
        ar = bytes_to_uint('-', 'd', 't', 's');
        br = bytes_to_uint('-', 'u', 'p', 'c');
        cr = bytes_to_uint('-', 'd', 't', 's');
        dr = bytes_to_uint('.', 'u', 'p', 'c');
        s_cycles = 0;
    }
}
#pragma region DATA_MANAGEMENT
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
    UCPL
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
    NCPL(0)
    reg = arg0;
    s_cycles = 0;
}
template<reg16& reg> static void i_sseg()
{
    UCPL
   store_arg(arg0_a, arg0_type(), reg);
   s_cycles = 0;
}
#pragma endregion
#pragma region INTERRUPTS
static void i_lihp()
{
    NCPL(0)
    ihpr = arg0;
    s_cycles = 0;
    return;
}
static void i_sihp()
{
    UCPL
    store_arg(arg0_a, arg0_type(), ihpr);
    s_cycles = 0;
    return;
}
static void i_int()
{
    s_software_interrupt_req = true;
    s_software_interrupt_code = arg0;
    s_cycles = 0;
}
static void i_iret()
{
    static uint adr = 0;
    switch(s_cycles)
    {
#define LOAD4(n, r, pr) case n: pr = s_mem->data; READ_MEM(adr, DWORD, s_mem); adr -= 4; s_cycles++; return
#define LOAD2(n, r, pr) case n: pr = s_mem->data; READ_MEM(adr, WORD , s_mem); adr -= 2; s_cycles++; return
#define LOAD1(n, r, pr) case n: pr = s_mem->data; READ_MEM(adr, BYTE , s_mem); adr -= 1; s_cycles++; return
    case 1:
        READ_MEM(0x37FFFFF, DWORD, s_mem);
        adr = 0x037FFFFB;
        s_cycles = 2;
        return;
    LOAD4(2, br, ar);
    LOAD4(3 , cr   , br);
    LOAD4(4 , dr   , cr);
    LOAD4(5 , rr   , dr);
    LOAD4(6 , fr   , rr);
    LOAD4(7 , tr   , fr);
    LOAD4(8 , ur   , tr);
    LOAD4(9 , spr  , ur);
    LOAD2(10, dsr  , spr);
    LOAD2(11, csr  , dsr);
    LOAD2(12, ssr  , csr);
    LOAD4(13, rpr  , ssr);
    LOAD4(14, ipr  , rpr);
    LOAD4(15, cr0  , ipr);
    LOAD4(16, cr1  , cr0);
    LOAD4(17, cr2  , cr1);
    LOAD4(18, far  , cr2);
    LOAD4(19, fbr  , far);
    LOAD4(20, fcr  , fbr);
    LOAD4(21, fdr  , fcr);
    LOAD4(22, frr  , fdr);
    LOAD1(23, cplr , frr);
    case 24:
        cplr = s_mem->data;
        s_cycles = 0;
        return;
#undef LOAD1
#undef LOAD2
#undef LOAD4
    }
}
#pragma endregion
#pragma region OPERATORS
#define DEF_OPERATOR_INS(n, e) static void i_##n() \
                               {\
                                   store_arg(arg0_a, arg0_type(), e);\
                                   s_cycles = 0;\
                               }

DEF_OPERATOR_INS(not , ~(arg0        ))
DEF_OPERATOR_INS(or  ,  (arg0 |  arg1))
DEF_OPERATOR_INS(and ,  (arg0 &  arg1))
DEF_OPERATOR_INS(xor ,  (arg0 ^  arg1))
DEF_OPERATOR_INS(nor , ~(arg0 |  arg1))
DEF_OPERATOR_INS(nand, ~(arg0 &  arg1))
DEF_OPERATOR_INS(xnor, ~(arg0 ^  arg1))
DEF_OPERATOR_INS(add ,  (arg0 +  arg1))
DEF_OPERATOR_INS(sub ,  (arg0 -  arg1))
DEF_OPERATOR_INS(mul ,  (arg0 *  arg1))
DEF_OPERATOR_INS(div ,  (arg0 /  arg1))
DEF_OPERATOR_INS(sl  ,  (arg0 << arg1))
DEF_OPERATOR_INS(sr  ,  (arg0 >> arg1))

DEF_OPERATOR_INS(fadd,  float_as_uint((uint_as_float(arg0) +  uint_as_float(arg1))));
DEF_OPERATOR_INS(fsub,  float_as_uint((uint_as_float(arg0) -  uint_as_float(arg1))));
DEF_OPERATOR_INS(fmul,  float_as_uint((uint_as_float(arg0) *  uint_as_float(arg1))));
DEF_OPERATOR_INS(fdiv,  float_as_uint((uint_as_float(arg0) /  uint_as_float(arg1))));

#undef DEF_OPERATOR_INS
#pragma endregion
#pragma region EXECUTION_FLOW
static void i_jmp()
{
    ipr = arg0;
    s_cycles = 0;
}
static void i_call()
{
    rpr = ipr;
    ipr = arg0;
    s_cycles = 0;
}
static void i_cmp()
{
    if((int) arg0 == 0)
        set_flag(flag::Z);
    if((int) arg0 == (int) arg1)
        set_flag(flag::E);
    if((int) arg0 > (int) arg1)
        set_flag(flag::G);
    if((int) arg0 < (int) arg1)
        set_flag(flag::L);
    s_cycles = 0;
}
static void i_fcmp()
{
    if(uint_as_float(arg0) == 0)
        set_flag(flag::Z);
    if(uint_as_float(arg0) == uint_as_float(arg1))
        set_flag(flag::E);
    if(uint_as_float(arg0) > uint_as_float(arg1))
        set_flag(flag::G);
    if(uint_as_float(arg0) < uint_as_float(arg1))
        set_flag(flag::L);
    s_cycles = 0;
}
static void i_ucmp()
{
    if(arg0 == 0)
        set_flag(flag::Z);
    if(arg0 == arg1)
        set_flag(flag::E);
    if(arg0 > arg1)
        set_flag(flag::G);
    if(arg0 < arg1)
        set_flag(flag::L);
    s_cycles = 0;
}

#define DEF_CJMP(n, e) static void i_j##n()\
                       {\
                           if(flag_on((flag) e))\
                               ipr = arg0;\
                           s_cycles = 0;\
                       }

DEF_CJMP(e, flag::E)
DEF_CJMP(z, flag::Z);
DEF_CJMP(g, flag::G)
DEF_CJMP(l, flag::L)
DEF_CJMP(eg, (flag::E | flag::G))
DEF_CJMP(el, (flag::E | flag::L))

#undef DEF_CJMP

static void i_ret()
{
    ipr = rpr;
    s_cycles = 0;
}
static void i_hlt()
{
    UCPL
    s_dc.fp_shutdown(0, "CPU halted");
    s_cycles = 0;
    return;
}

#define DEF_SLEEP(n, m) static void i_sleep##n()\
                        {\
                            s_sleep_time = arg0;\
                            s_sleep = true;\
                            s_sleep_mode = sleep_mode::m;\
                            /* maybe do this on another thread*/\
                            if(s_sleep_mode == sleep_mode::S)\
                                std::this_thread::sleep_for(std::chrono::duration<long double, std::ratio<1, 1>>(s_sleep_time));\
                            else if(s_sleep_mode == sleep_mode::MS)\
                                std::this_thread::sleep_for(std::chrono::duration<long double, std::milli>(s_sleep_time));\
                            else if(s_sleep_mode == sleep_mode::MCS)\
                                std::this_thread::sleep_for(std::chrono::duration<long double, std::micro>(s_sleep_time));\
                            s_sleep = false;\
                            s_cycles = 0;\
                        }

DEF_SLEEP(s, S)
DEF_SLEEP(ms, MS)
DEF_SLEEP(mcs, MCS)

#pragma endregion
#pragma region SECURITY
static void i_lcpl()
{
    NCPL(0)
    cplr = arg0;
    s_cycles = 0;
}
static void i_scpl()
{
    UCPL
    store_arg(arg0_a, arg0_type(), cplr);
    s_cycles = 0;
}
#pragma endregion
#pragma region IO
static void i_in()
{
    if(!s_use_io_bus)
    {
        s_cycles = 0;
        return;
    }
    switch(s_cycles)
    {
    case 1:
        sp_io_bus->address = arg0;
        s_cycles = 2;
        return;
    case 2:
        BREAKPOINT("Check arg1_a");
        store_arg(arg1_a, arg1_type(), sp_io_bus->value);
        s_cycles = 0;
        return;
    }
}
static void i_out()
{
    sp_io_bus->value = arg1;
    sp_io_bus->address = arg0;
    s_cycles = 0;
}
#pragma endregion

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
        BREAKPOINT("interrupt_setup: cycle " << s_cycles);
        static uint adr = 0;
        switch (s_cycles)
        {
#define SAVE4(n, v) case n: WRITE_MEM(adr, v, DWORD, s_mem); s_cycles++; adr -= 4; return
#define SAVE2(n, v) case n: WRITE_MEM(adr, v, WORD , s_mem); s_cycles++; adr -= 2; return
#define SAVE1(n, v) case n: WRITE_MEM(adr, v, BYTE , s_mem); s_cycles++; adr -= 1; return
        case 0:
            WRITE_MEM(0x037FFFFF, ar, DWORD, s_mem);
            adr = 0x037FFFFB;
            s_cycles++;
            return;
        SAVE4(1 , br  );
        SAVE4(2 , cr  );
        SAVE4(3 , dr  );
        SAVE4(4 , rr  );
        SAVE4(5 , fr  );
        SAVE4(6 , tr  );
        SAVE4(7 , ur  );
        SAVE4(8 , spr );
        SAVE2(9,  dsr );
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
            fr   = DEFAULT_FLAGS        ;
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
                INSTRUCTION(LIHP    , i_lihp        );
                INSTRUCTION(SIHP    , i_sihp        );
                INSTRUCTION(INT     , i_int         );
                INSTRUCTION(IRET    , i_iret        );
                INSTRUCTION(NOT     , i_not         );
                INSTRUCTION(OR      , i_or          );
                INSTRUCTION(AND     , i_and         );
                INSTRUCTION(XOR     , i_xor         );
                INSTRUCTION(NOR     , i_nor         );
                INSTRUCTION(NAND    , i_nand        );
                INSTRUCTION(XNOR    , i_xnor        );
                INSTRUCTION(ADD     , i_add         );
                INSTRUCTION(FADD    , i_fadd        );
                INSTRUCTION(SUB     , i_sub         );
                INSTRUCTION(FSUB    , i_fsub        );
                INSTRUCTION(MUL     , i_mul         );
                INSTRUCTION(FMUL    , i_fmul        );
                INSTRUCTION(DIV     , i_div         );
                INSTRUCTION(FDIV    , i_fdiv        );
                INSTRUCTION(SL      , i_sl          );
                INSTRUCTION(SR      , i_sr          );
                INSTRUCTION(JMP     , i_jmp         );
                INSTRUCTION(CALL    , i_call        );
                INSTRUCTION(CMP     , i_cmp         );
                INSTRUCTION(FCMP    , i_fcmp        );
                INSTRUCTION(UCMP    , i_ucmp        );
                INSTRUCTION(JE      , i_je          );
                INSTRUCTION(JZ      , i_jz          );
                INSTRUCTION(JG      , i_jg          );
                INSTRUCTION(JL      , i_jl          );
                INSTRUCTION(JEG     , i_jeg         );
                INSTRUCTION(JEL     , i_jel         );
                INSTRUCTION(RET     , i_ret         );
                INSTRUCTION(HLT     , i_hlt         );
                INSTRUCTION(SLEEPS  , i_sleeps      );
                INSTRUCTION(SLEEPMS , i_sleepms     );
                INSTRUCTION(SLEEPMCS, i_sleepmcs    );
                INSTRUCTION(LCPL    , i_lcpl        );
                INSTRUCTION(SCPL    , i_scpl        );
                INSTRUCTION(IN      , i_in          );
                INSTRUCTION(OUT     , i_out         );
#undef INSTRUCTION
            }
        }
    }
}
__export void module_destroy_device(device_ptr i)
{
    std::cout << std::showbase << std::hex;
    LOG_M("Cpu state: ");
    LOG_M("\ta      : " << ar   << "\tb : " << br);
    LOG_M("\tc      : " << cr   << "\td : " << dr);
    LOG_M("\tds     : " << dsr  << "\tcs: " << csr << "\tss: " << ssr);
    LOG_M("\tsp     : " << spr  << "\tmemr0: " << memr0 << "\tmemr1: " << memr1);
    LOG_M("\tihp    : " << ihpr << "\tirp: " << irpr);
    LOG_M("\tip     : " << ipr  << "\trp: " << rpr);
    LOG_M(std::noshowbase);
    LOG_M("\tflags  : ");
    LOG_M("\t\tE:   : " << flag_on(flag::E));
    LOG_M("\t\tZ:   : " << flag_on(flag::Z));
    LOG_M("\t\tG:   : " << flag_on(flag::G));
    LOG_M("\t\tL:   : " << flag_on(flag::L));
    LOG_M("\t\tI:   : " << flag_on(flag::I));
    LOG_M(std::showbase);
    LOG_M("\tGDTR   : offset: " << gdtr.offset << ", base: " << gdtr.address);
    LOG_M("\tt      : " << tr   << "\tu : " << ur);
    std::cout << std::noshowbase << std::dec;
}
