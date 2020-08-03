#include <dotcvm/dotcvm.hpp>
#include <dotcvm/utils/log.hpp>
#include <modules/memory.hpp>
#include <modules/io_bus.hpp>


static memory* s_instance;

static uint8_t* s_data;
static uint32_t s_size;

static bool s_use_io_bus = true;
static io_bus*  sp_io_bus = nullptr;

__export void module_report(uint additional_data0, uint additional_data1)
{
    if((additional_data0 == DC_CONNECTION_INAGREEMENT || additional_data0 == DC_CONNECTION_INEXISTANT) && additional_data1 == DC_STD_IOB)
    {
        warn("Reported that io bus doesn't exist");
        s_use_io_bus = false;
    }
}

__export device_ptr module_create_device(dotcvm_data d)
{

    if(s_use_io_bus)
        sp_io_bus = (io_bus*) d.fp_get_device(DC_STD_IOB);
    config c = d.fp_read_module_config("conf.cfg");
    s_size = d.fp_config_get_uint(c, "size", MEMORY_DEFAULT_SIZE);
    debug("Memory size: " << s_size);
    s_data = new uint8_t[s_size];
    s_instance = new memory;
    s_instance->address = 0;
    s_instance->data = s_data[0]; // to be accurate
    s_instance->write = false;
    s_instance->mode = memory_mode::BYTE;
    return s_instance;
}

__export void module_clock()
{
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
            s_instance->data = *(uint16_t*)(&(s_data[s_instance->address]));
        else if(s_instance->mode == memory_mode::DWORD)
            s_instance->data = *(uint32_t*)(&(s_data[s_instance->address]));
    }
}

__export void module_destroy_device(device_ptr i)
{
    delete s_instance;
    delete[] s_data;
}
