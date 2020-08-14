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

// those are variables that instructions may use and redifine them
static uint arg0    = 0; // the value of arg0
static uint arg0_a  = 0; // the address of arg0
static uint arg1    = 0; // the same for arg1
static uint arg1_a  = 0;

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

static uint32_t s_interrupt_code = 0;

enum flag { Z = (1 << 0), // On if cmp arg0 = 0
            E = (1 << 1), // On if cmp arg0 = arg1
            G = (1 << 2), // on if cmp arg0 > arg1
            L = (1 << 3), // on if cmp arg0 < arg1
            I = (1 << 4)  // set by user with instruction cli abd sci // if set then interrupts are enabled
             };

static bool s_on_interrupt = false;
static bool s_software_interrupt_request = false;
static uint s_software_interrupt_code = 0;

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
static void load_arg0(uint next_cycle)
{
    switch(arg0_type())
    {
    case arg_type::REG:
    case arg_type::LITERAL8:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg0_a = ipr;
        ipr += 1;
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL16:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg0_a = ipr;
        ipr += 2;
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL32:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg0_a = ipr;
        ipr += 4;
        s_cycles = next_cycle;
        return;
    case arg_type::MEMORY8:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg0_a = memr0;
        s_cycles = next_cycle;
        return;
    case arg_type::MEMORY16:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg0_a = memr0;
        s_cycles = next_cycle;
        return;
    case arg_type::MEMORY32:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg0_a = memr0;
        s_cycles = next_cycle;
        return;
    }
}
static void load_arg1(uint next_cycle)
{
    switch(arg1_type())
    {
    case arg_type::REG:
    case arg_type::LITERAL8:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg1_a = ipr;
        ipr += 1;
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL16:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg1_a = ipr;
        ipr += 2;
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL32:
        READ_MEM(c_adr(ipr), BYTE, s_mem);
        arg1_a = ipr;
        ipr += 4;
        s_cycles = next_cycle;
        return;
    case arg_type::MEMORY8:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg1_a = memr0;
        s_cycles = next_cycle;
        return;
    case arg_type::MEMORY16:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg1_a = memr0;
        s_cycles = next_cycle;
        return;
    case arg_type::MEMORY32:
        READ_MEM(d_adr(memr0), BYTE, s_mem);
        arg1_a = memr0;
        s_cycles = next_cycle;
        return;
    }
}
static void store_arg0(uint v, uint adr, uint next_cycle)
{
    switch(arg0_type())
    {
    case arg_type::REG:
        get_reg(adr) = v;
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL8:
    case arg_type::MEMORY8:
        WRITE_MEM(adr, v, BYTE, s_mem);
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL16:
    case arg_type::MEMORY16:
        WRITE_MEM(adr, v, WORD, s_mem);
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL32:
    case arg_type::MEMORY32:
        WRITE_MEM(adr, v, DWORD, s_mem);
        s_cycles = next_cycle;
        return;
    }
}
static void store_arg1(uint v, uint adr, uint next_cycle)
{
    switch(arg1_type())
    {
    case arg_type::REG:
        get_reg(adr) = v;
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL8:
    case arg_type::MEMORY8:
        WRITE_MEM(adr, v, BYTE, s_mem);
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL16:
    case arg_type::MEMORY16:
        WRITE_MEM(adr, v, WORD, s_mem);
        s_cycles = next_cycle;
        return;
    case arg_type::LITERAL32:
    case arg_type::MEMORY32:
        WRITE_MEM(adr, v, DWORD, s_mem);
        s_cycles = next_cycle;
        return;
    }
}


#pragma region INSTRUCTIONS

static void i_cpuinf()
{
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
}
static void i_set()
{
    // TODO: redo set instruction I am not proud of it
    switch (s_cycles)
    {
    case 1: // read arg0
        switch (arg0_type())
        {
        case arg_type::REG:
            READ_MEM(c_adr(ipr), BYTE, s_mem);
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
            READ_MEM(c_adr(ipr), BYTE, s_mem);
            ipr += 1;
            s_cycles = 4;
            return;
        case arg_type::LITERAL16:
            READ_MEM(c_adr(ipr), WORD, s_mem);
            ipr += 2;
            s_cycles = 4;
            return;
        case arg_type::LITERAL32:
            READ_MEM(c_adr(ipr), DWORD, s_mem);
            ipr += 4;
            s_cycles = 4;
            return;
        case arg_type::MEMORY8:
            READ_MEM(d_adr(memr1), BYTE, s_mem);
            s_cycles = 4;
            return;
        case arg_type::MEMORY16:
            READ_MEM(d_adr(memr1), WORD, s_mem);
            s_cycles = 4;
            return;
        case arg_type::MEMORY32:
            READ_MEM(d_adr(memr1), DWORD, s_mem);
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
            WRITE_MEM(c_adr(arg0), arg1, BYTE, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY8:
            WRITE_MEM(d_adr(arg0), arg1, BYTE, s_mem);
            s_cycles = 0;
            return;
        case arg_type::LITERAL16:
            WRITE_MEM(c_adr(arg0), arg1, WORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY16:
            WRITE_MEM(d_adr(arg0), arg1, WORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::LITERAL32:
            WRITE_MEM(c_adr(arg0), arg1, DWORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY32:
            WRITE_MEM(d_adr(arg0), arg1, DWORD, s_mem);
            s_cycles = 0;
            return;
        }
    }
}
static void i_push()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch (arg0_type())
        {
        case arg_type::REG:
            WRITE_MEM(s_adr(spr), get_reg(s_mem->data), DWORD, s_mem);
            spr -= 4;
            s_cycles = 0;
            return;
        case arg_type::MEMORY8:
        case arg_type::LITERAL8:
            WRITE_MEM(s_adr(spr), s_mem->data, BYTE, s_mem);
            spr -= 1;
            s_cycles = 0;
            return;
        case arg_type::MEMORY16:
        case arg_type::LITERAL16:
            WRITE_MEM(s_adr(spr), s_mem->data, WORD, s_mem);
            spr -= 2;
            s_cycles = 0;
            return;
        case arg_type::MEMORY32:
        case arg_type::LITERAL32:
            WRITE_MEM(s_adr(spr), s_mem->data, DWORD, s_mem);
            spr -= 4;
            s_cycles = 0;
            return;
        }
    }
}
static void i_pop()
{
    switch (s_cycles)
    {
    case 1:
        switch(arg0_type())
        {
        case arg_type::REG:
        case arg_type::LITERAL32:
        case arg_type::MEMORY32:
            spr += 4;
            READ_MEM(s_adr(spr), DWORD, s_mem);
            s_cycles = 2;
            return;
        case arg_type::LITERAL16:
        case arg_type::MEMORY16:
            spr += 2;
            READ_MEM(s_adr(spr), WORD, s_mem);
            s_cycles = 2;
            return;
        case arg_type::LITERAL8:
        case arg_type::MEMORY8:
            spr += 1;
            READ_MEM(s_adr(spr), BYTE, s_mem);
            s_cycles = 2;
            return;
        }
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            arg0 = s_mem->data;
            READ_MEM(c_adr(ipr), BYTE, s_mem);
            ipr += 1;
            s_cycles = 3;
            return;
        case arg_type::LITERAL8:
            WRITE_MEM(c_adr(ipr), s_mem->data, BYTE, s_mem);
            ipr += 1;
            s_cycles = 0;
            return;
        case arg_type::LITERAL16:
            WRITE_MEM(c_adr(ipr), s_mem->data, WORD, s_mem);
            ipr += 2;
            s_cycles = 0;
            return;
        case arg_type::LITERAL32:
            WRITE_MEM(c_adr(ipr), s_mem->data, DWORD, s_mem);
            ipr += 4;
            s_cycles = 0;
            return;
        case arg_type::MEMORY8:
            WRITE_MEM(d_adr(memr0), s_mem->data, BYTE, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY16:
            WRITE_MEM(d_adr(memr0), s_mem->data, WORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY32:
            WRITE_MEM(d_adr(memr0), s_mem->data, DWORD, s_mem);
            s_cycles = 0;
            return;
        }
    case 3:
        get_reg(s_mem->data) = arg0;
        s_cycles = 0;
        return;
    }
}
static void i_lgdtr()
{
    if(cplr != 0)
    {
        // TODO: fire an interrupt here
        s_cycles = 0;
        return;
    }
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch (arg0_type())
        {
        case arg_type::REG:
            arg0 = get_reg(s_mem->data);
            READ_MEM(arg0, WORD, s_mem);
            s_cycles = 3;
            return;
        case arg_type::LITERAL8:
        case arg_type::LITERAL16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY8:
        case arg_type::MEMORY16:
        case arg_type::MEMORY32:
            arg0 = s_mem->data;
            READ_MEM(arg0, WORD, s_mem);
            s_cycles = 3;
            return;
        }
    case 3:
        arg1 = s_mem->data;
        READ_MEM(arg0 + 2, DWORD, s_mem);
        s_cycles = 4;
        return;
    case 4:
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
        load_arg0(2);
        return;
    case 2:
        switch (arg0_type())
        {
        case arg_type::REG:
            arg0 = get_reg(s_mem->data);
            WRITE_MEM(d_adr(arg0), gdtr.offset, WORD, s_mem);
            s_cycles = 3;
            return;
        case arg_type::LITERAL8:
        case arg_type::LITERAL16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY8:
        case arg_type::MEMORY16:
        case arg_type::MEMORY32:
            arg0 = s_mem->data;
            WRITE_MEM(d_adr(arg0), gdtr.address, WORD, s_mem);
            s_cycles = 3;
            return;
        }
    case 3:
        WRITE_MEM(d_adr(arg0 + 2), gdtr.address, DWORD, s_mem);
        s_cycles = 0;
        return;
    }
}
template<reg16& seg> static void i_lsegment()
{
    if(cplr != 0)
    {
        // TODO: fire an interrupt here
        s_cycles = 0;
        return;
    }
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            READ_MEM(get_reg(s_mem->data), WORD, s_mem);
            s_cycles = 3;
            return;
        case arg_type::LITERAL8:
        case arg_type::MEMORY8:
        case arg_type::LITERAL16:
        case arg_type::MEMORY16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY32:
            READ_MEM(s_mem->data, WORD, s_mem);
            s_cycles = 3;
            return;
        }
    case 3:
        seg = s_mem->data;
        s_cycles = 0;
        return;
    }
}
template<reg16& seg> static void i_ssegment()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            WRITE_MEM(d_adr(get_reg(s_mem->data)), seg, WORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::LITERAL8:
        case arg_type::LITERAL16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY8:
        case arg_type::MEMORY16:
        case arg_type::MEMORY32:
            WRITE_MEM(d_adr(s_mem->data), seg, WORD, s_mem);
            s_cycles = 0;
            return;
        }
    }
}
static void i_lihp()
{
    if(cplr != 0)
    {
        // TODO: fire an interrupt here
        s_cycles = 0;
        return;
    }
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            ihpr = get_reg(s_mem->data);
            s_cycles = 0;
            return;
        case arg_type::LITERAL8:
        case arg_type::LITERAL16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY8:
        case arg_type::MEMORY16:
        case arg_type::MEMORY32:
            ihpr = s_mem->data;
            s_cycles = 0;
            return;
        }
    }
}
static void i_sihp()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            WRITE_MEM(d_adr(get_reg(s_mem->data)), ihpr, DWORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::LITERAL8:
        case arg_type::LITERAL16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY8:
        case arg_type::MEMORY16:
        case arg_type::MEMORY32:
            WRITE_MEM(d_adr(s_mem->data), ihpr, DWORD, s_mem);
            s_cycles = 0;
            return;
        }
    }
}
static void i_int()
{
    if(s_on_interrupt)
    {
        // TODO: fire an interrupt here
        s_cycles = 0;
        return;
    }
    if(cplr != 0)
    {
        // TODO: fire an interrupt here
        s_cycles = 0;
        return;
    }
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            s_software_interrupt_code = get_reg(s_mem->data);
            s_software_interrupt_request = true;
            s_cycles = 0;
            return;
        case arg_type::LITERAL8:
        case arg_type::LITERAL16:
        case arg_type::LITERAL32:
        case arg_type::MEMORY8:
        case arg_type::MEMORY16:
        case arg_type::MEMORY32:
            DEBUG_M("This is the second cycle");
            s_software_interrupt_code = s_mem->data;
            s_software_interrupt_request = true;
            s_cycles = 0;
            return;
        }
    }
}
static void i_iret()
{
    static uint adr = 0;
    switch(s_cycles)
    {
#define LOAD4(n, r, pr) case n: pr = s_mem->data; READ_MEM(adr, DWORD, s_mem); adr -= 4; s_cycles++; return
#define LOAD2(n, r, pr) case n: pr = s_mem->data; READ_MEM(adr, WORD , s_mem); adr -= 2; s_cycles++; return
#define LOAD1(n, r, pr) case n: pr = s_mem->data; READ_MEM(adr, BYTE , s_mem); adr -= 1; s_cycles++; return
    case 1: // a reg
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
static void i_not()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        switch(arg0_type())
        {
        case arg_type::REG:
            get_reg(s_mem->data) = ~(get_reg(s_mem->data));
            s_cycles = 0;
            return;
        case arg_type::LITERAL8:
            WRITE_MEM(c_adr(ipr - 1), ~(s_mem->data), BYTE, s_mem);
            s_cycles = 0;
            return;
        case arg_type::LITERAL16:
            WRITE_MEM(c_adr(ipr - 2), ~(s_mem->data), WORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::LITERAL32:
            WRITE_MEM(c_adr(ipr - 4), ~(s_mem->data), DWORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY8:
            WRITE_MEM(d_adr(memr0), ~(s_mem->data), BYTE, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY16:
            WRITE_MEM(d_adr(memr0), ~(s_mem->data), WORD, s_mem);
            s_cycles = 0;
            return;
        case arg_type::MEMORY32:
            WRITE_MEM(d_adr(memr0), ~(s_mem->data), DWORD, s_mem);
            s_cycles = 0;
            return;
        }

    }
}
static void i_or()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 | arg1, arg0_a, 0);
        return;
    }
}
static void i_and()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 & arg1, arg0_a, 0);
        return;
    }
}
static void i_xor()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 ^ arg1, arg0_a, 0);
        return;
    }
}
static void i_nor()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(~(arg0 | arg1), arg0_a, 0);
        return;
    }
}
static void i_nand()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(~(arg0 & arg1), arg0_a, 0);
        return;
    }
}
static void i_xnor()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(~(arg0 ^ arg1), arg0_a, 0);
        return;
    }
}
static void i_add()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            BREAKPOINT("Check s_mem->data");
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 + arg1, arg0_a, 0);
        return;
    }
}
static void i_fadd()
{
    // TODO: implement fadd
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 + arg1, arg0_a, 0);
        return;
    }
}
static void i_sub()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 - arg1, arg0_a, 0);
        return;
    }
}
static void i_fsub()
{
    // TODO: implement fsub
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 - arg1, arg0_a, 0);
        return;
    }
}
static void i_mul()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 * arg1, arg0_a, 0);
        return;
    }
}
static void i_fmul()
{
    // TODO: implement fmul
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 - arg1, arg0_a, 0);
        return;
    }
}
static void i_div()
{
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 / arg1, arg0_a, 0);
        return;
    }
}
static void i_fdiv()
{
    // TODO: implement fdiv
    switch(s_cycles)
    {
    case 1:
        load_arg0(2);
        return;
    case 2:
        if(arg0_type() == arg_type::REG)
        {
            arg0_a = s_mem->data;
            arg0 = get_reg(s_mem->data);
        }
        else
            arg0 = s_mem->data;  
        load_arg1(3);
        return;
    case 3:
        if(arg1_type() == arg_type::REG)
        {
            arg1_a = s_mem->data;
            arg1 = get_reg(s_mem->data);
        }
        else
            arg1 = s_mem->data;
        store_arg0(arg0 - arg1, arg0_a, 0);
        return;
    }
}


static void i_hlt()
{
    s_dc.fp_shutdown(0, "CPU halted");
    s_cycles = 0;
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
    if(s_cycles == 0)
    {
        if(s_interrupt)
            s_interrupt = false;
        if((sp_interrupt_bus->mode == interrupt_mode::REQUEST && flag_on(flag::I) && !s_on_interrupt) || (sp_interrupt_bus->mode == interrupt_mode::FORCE && !s_on_interrupt))
        {
            s_on_interrupt = true;
            s_interrupt = true;
            s_interrupt_code = sp_interrupt_bus->interrupt_code;
            s_cycles = 1;
        }
        else if(s_software_interrupt_request && !s_on_interrupt)
        {
            s_software_interrupt_request = false;
            s_on_interrupt = true;
            s_interrupt = true;
            s_cycles = 1;
            s_interrupt_code = s_software_interrupt_code;
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
#define INSTRUCTION(i, f) case instruction::i: DEBUG_M("Instruction: " << #i << ", cycle: " << s_cycles); f(); return
        INSTRUCTION(CPUINF, i_cpuinf);
        INSTRUCTION(SET, i_set);
        INSTRUCTION(PUSH, i_push);
        INSTRUCTION(POP, i_pop);
        INSTRUCTION(LGDTR, i_lgdtr);
        INSTRUCTION(SGDTR, i_sgdtr);
        INSTRUCTION(LDS, i_lsegment<dsr>);
        INSTRUCTION(LCS, i_lsegment<csr>);
        INSTRUCTION(LSS, i_lsegment<ssr>);
        INSTRUCTION(SDS, i_ssegment<dsr>);
        INSTRUCTION(SCS, i_ssegment<csr>);
        INSTRUCTION(SSS, i_ssegment<ssr>);
        INSTRUCTION(LIHP, i_lihp);
        INSTRUCTION(SIHP, i_sihp);
        INSTRUCTION(INT, i_int);
        INSTRUCTION(IRET, i_iret);
        INSTRUCTION(NOT, i_not);
        INSTRUCTION(OR, i_or);
        INSTRUCTION(AND, i_and);
        INSTRUCTION(XOR, i_xor);
        INSTRUCTION(NOR, i_nor);
        INSTRUCTION(NAND, i_nand);
        INSTRUCTION(XNOR, i_xnor);
        INSTRUCTION(ADD, i_add);
        INSTRUCTION(FADD, i_fadd);
        INSTRUCTION(SUB, i_sub);
        INSTRUCTION(FSUB, i_fsub);
        INSTRUCTION(MUL, i_mul);
        INSTRUCTION(FMUL, i_fmul);
        INSTRUCTION(DIV, i_div);
        INSTRUCTION(FDIV, i_fdiv);

        INSTRUCTION(HLT, i_hlt);
#undef INSTRUCTION
        case instruction::NO_OP:
            DEBUG_M("Instruction NO_OP");
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
    LOG_M("\ta  : " << ar  <<  "\tb  : " << br);
    LOG_M("\tc  : " << cr  <<  "\td  : " << dr);
    LOG_M("\tds : " << dsr <<  "\tcs : " << csr << "\tss: " << ssr);
    LOG_M("\tihp: " << ihpr << "\tirp: " << irpr);
    LOG_M("\tip: " << ipr <<  "\trp : " << rpr);
    LOG_M("\tt  : " << tr  <<  "\tu  : " << ur);
#endif
}
