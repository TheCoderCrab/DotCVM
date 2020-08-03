#ifndef DC_MEMORY_H
#define DC_MEMORY_H

#include <cstdint>

#define DC_STR_MEM                  0xDC57D3E3
#define MEMORY_DEFAULT_SIZE         (50 * 1024 * 1024)

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