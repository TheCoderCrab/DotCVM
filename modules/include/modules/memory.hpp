#ifndef DC_MEMORY_H
#define DC_MEMORY_H

#include <cstdint>

#define DC_STD_MEM                  0xDC57D3E3
#define MEMORY_DEFAULT_SIZE         (64 * 1024 * 1024)

#define WRITE_MEM(a, d, m, i)       i->address = a; i->data = d; i->mode = memory_mode::m; i->write = true
#define READ_MEM(a, m, i)           i->address = a; i->mode = memory_mode::m; i->write = false

#define WRITE_MEM_E(a, d, m, i)     i->address = a; i->data = d; i->mode = m; i->write = true
#define READ_MEM_E(a, m, i)         i->address = a; i->mode = m; i->write = false

enum memory_mode {  BYTE  = 1,
                    WORD  = 2,
                    DWORD = 4 };

struct memory
{
    uint32_t        address;
    uint32_t        data;
    bool            write;
    memory_mode     mode;
};

#endif /* DC_MEMORY_H */