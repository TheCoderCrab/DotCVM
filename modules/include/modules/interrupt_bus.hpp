#ifndef DC_INTERRUPT_BUS_H
#define DC_INTERRUPT_BUS_H

#include <cstdint>

enum interrupt_mode { NONE, REQUEST, FORCE };

struct interrupt_bus
{
    interrupt_mode      mode;
    uint32_t            interrupt_code;
};

#endif /* DC_INTERRUPT_BUS_H */