#include <log.h>
#include <app_main.h>
#include <devices.h>
#include <main.h>


void cmain()
{
    log("Starting DotC Virtual Machine");
    devices::init(MEM_SIZE, DISK_SIZE);
}

void update()
{
    devices::update();
}

void exit()
{
    log("Closing DotC Virtual Machine");
    devices::exit();
    log("Bye");
}
