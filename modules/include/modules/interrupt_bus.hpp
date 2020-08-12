#ifndef DC_INTERRUPT_BUS_H
#define DC_INTERRUPT_BUS_H

#include <cstdint>

#define DC_STD_ITB      0xDC57D17B

enum interrupt_mode { NONE, REQUEST, FORCE };

struct interrupt_bus
{
    interrupt_mode      mode;
    uint32_t            interrupt_code;
};

struct interrupt_data
{
    uint interrupt_code;
    interrupt_mode mode;
    uint data0;
    uint data1;
};

#endif /* DC_INTERRUPT_BUS_H */