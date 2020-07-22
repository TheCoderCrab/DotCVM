#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/log.h>

namespace devices
{
    CPU* cpu;
    void init(uint32_t memSize, uint32_t diskSizeIfNotPresentInBlocks)
    {
        log("Initializing devices");
        cpu = new CPU(memSize, diskSizeIfNotPresentInBlocks);
        debug("Created CPU");
    }
    void update()
    {
        cpu->update();
    }

    void exit()
    {
        delete cpu;
    }

}
