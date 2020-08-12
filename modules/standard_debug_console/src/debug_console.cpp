#include <dotcvm/dotcvm.hpp>
#include <dotcvm/utils/log.hpp>
#include <modules/io_bus.hpp>
#include <modules/debug_console.hpp>
#include <iostream>

static dotcvm_data s_dc;
static io_bus* sp_io_bus;

__export device_ptr module_create_device(dotcvm_data d)
{
    s_dc = d;
    return nullptr;
}

__export void module_init()
{
    sp_io_bus = (io_bus*) s_dc.fp_get_device(DC_STD_IOB);
}

__export void module_post_clock(uint cycles)
{
    if(sp_io_bus->address == DC_STD_DBC)
    {
        std::cout << (char) sp_io_bus->value;
        std::cout.flush();
    }
}

__export void module_destroy_device(device_ptr i)
{

}

