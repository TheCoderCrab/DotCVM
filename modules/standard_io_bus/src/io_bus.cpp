#include <dotcvm/dotcvm.hpp>
#include <modules/io_bus.hpp>
#include <dotcvm/utils/log.hpp>

__export void module_report(uint additional_data0, uint additional_data1)
{
    WARN_M("IO BUS: Reported: " << additional_data0 << ':' << additional_data1);
}

__export device_ptr module_create_device(dotcvm_data d)
{
    io_bus* i = new io_bus;
    i->address = 0;
    i->value = 0;
    return i;
}


__export void module_destroy_device(device_ptr i)
{
    delete (io_bus*) i;
}