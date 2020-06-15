#include <devices.h>
#include <log.h>

namespace devices
{
    Memory* mem;
    Drive* drive;
    Screen* scr;
    void init(unsigned int memSize, unsigned int diskSizeIfNotPresentInBlocks)
    {
        log("Initializing devices");
        mem = new Memory(memSize);
        drive = new Drive(diskSizeIfNotPresentInBlocks);
        scr = new Screen(mem);
    }
    void update()
    {
        debug("mem update");
        mem->update();
        debug("drive update");
        drive->update();
        debug("scr update");
        scr->update();
    }

    void exit()
    {
        log("Stopping devices");
        drive->exit();
        mem->exit();
        scr->exit();
        delete mem;
        delete drive;
        delete scr;

    }

}
