#include <dotcvm/dotcvm.hpp>
#include <dotcvm/utils/log.hpp>
#include <modules/memory.hpp>
#include <modules/io_bus.hpp>


static dotcvm_data s_dc;

static memory* s_instance;

static uint8_t* s_data;
static uint32_t s_size;

static bool s_use_io_bus = true;
static io_bus*  sp_io_bus = nullptr;

__export void module_report(uint additional_data0, uint additional_data1)
{
    if((additional_data0 == DC_CONNECTION_INAGREEMENT || additional_data0 == DC_CONNECTION_INEXISTANT) && additional_data1 == DC_STD_IOB)
    {
        WARN_M("MEMORY: Reported that io bus doesn't exist");
        s_use_io_bus = false;
    }
}

__export device_ptr module_create_device(dotcvm_data d)
{
    DEBUG_M("Creating memory");
    s_dc = d;
    config c = d.fp_read_module_config("conf.cfg");
    s_size = d.fp_config_get_uint(c, "size", MEMORY_DEFAULT_SIZE);
    DEBUG_M("Memory size: " << s_size << ",MB: " << s_size / 1024 / 1024);
    s_data = new uint8_t[s_size];
// This is a test program
#ifdef DEBUG
    uint a = 0;
#define PUT_B(b) s_data[a] = b; a += 1
    PUT_B(0x4C);
#undef PUT_B
#endif /* DEBUG */
    DEBUG_M("Memory allocated");
    if(s_size < 64 * 1024 * 1024)
        WARN_M("Memory size is smaller than 64MB this will create problems if you are using the standard cpu");
    s_instance = new memory;
    s_instance->address = 0;
    s_instance->data = s_data[0]; // to be accurate
    s_instance->write = false;
    s_instance->mode = memory_mode::BYTE;


    return s_instance;
}

__export void module_init()
{
    if(s_use_io_bus)
        sp_io_bus = (io_bus*) s_dc.fp_get_device(DC_STD_IOB);
}

__export void module_clock(uint cycles)
{
    if(s_instance->address >= s_size)
    {
        WARN_M("Memory: tried to access an out of range memory location: " << s_instance->address << " while size is: " << s_size);
        s_instance->address = s_instance->address % s_size;
    }
    if(s_instance->write)
    {
        if(s_instance->mode == memory_mode::BYTE)
            s_data[s_instance->address] = (uint8_t) s_instance->data;
        else if(s_instance->mode == memory_mode::WORD)
            *(uint16_t*)(&(s_data[s_instance->address])) = (uint16_t) s_instance->data;
        else if(s_instance->mode == memory_mode::DWORD)
            *(uint32_t*)(&(s_data[s_instance->address])) = s_instance->data;
    }
    else
    {
        if(s_instance->mode == memory_mode::BYTE)
            s_instance->data = s_data[s_instance->address];
        else if(s_instance->mode == memory_mode::WORD)
            s_instance->data = *((uint16_t*)(&(s_data[s_instance->address])));
        else if(s_instance->mode == memory_mode::DWORD)
            s_instance->data = *((uint32_t*)(&(s_data[s_instance->address])));
    }
}

__export void module_destroy_device(device_ptr i)
{
#ifdef DEBUG
    DEBUG_M((uint) s_data[0xFF]);
#endif /* DEBUG */
    delete s_instance;
    delete[] s_data;
}
