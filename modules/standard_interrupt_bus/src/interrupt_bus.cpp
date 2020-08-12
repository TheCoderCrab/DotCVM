#include <dotcvm/dotcvm.hpp>
#include <modules/interrupt_bus.hpp>
#include <dotcvm/utils/log.hpp>

__export void module_report(uint additional_data0, uint additional_data1)
{
    WARN_M("IO BUS: Reported: " << additional_data0 << ':' << additional_data1);
}

__export device_ptr module_create_device(dotcvm_data d)
{
    interrupt_bus* i = new interrupt_bus;
    i->interrupt_code = 0;
    i->mode = interrupt_mode::NONE;
    return i;
}


__export void module_destroy_device(device_ptr i)
{
    delete (interrupt_bus*) i;
}