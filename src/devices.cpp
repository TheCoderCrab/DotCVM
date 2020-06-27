#include <devices.h>
#include <log.h>

namespace devices
{
    CPU* cpu;
    void init(uint32_t memSize, uint32_t diskSizeIfNotPresentInBlocks)
    {
        log("Initializing devices");
        Memory* mem = new Memory(memSize);
        cpu = new CPU(mem, new Drive(diskSizeIfNotPresentInBlocks), new Screen(mem));
    }
    void update()
    {
        cpu->update();
    }

    void exit()
    {
        cpu->exit();
        delete cpu;
    }

}
