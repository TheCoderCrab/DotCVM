#include <devices.h>
#include <log.h>

namespace devices
{
    CPU* cpu;
    void init(uint32_t memSize, uint32_t diskSizeIfNotPresentInBlocks)
    {
        log("Initializing devices");
        Memory* mem = new Memory(memSize);
        debug("Memory allocated");
        cpu = new CPU(mem, new Drive(diskSizeIfNotPresentInBlocks), new ScreenDevice(mem));
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
