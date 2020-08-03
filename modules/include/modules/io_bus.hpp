#ifndef DC_IO_BUS_H
#define DC_IO_BUS_H

#include <cstdint>

#define DC_STD_IOB  0xDC57D10B

struct io_bus
{
    uint32_t    address;
    uint32_t    value;
};

#endif /* DC_IO_BUS_H */