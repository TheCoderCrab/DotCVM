#include <log.h>
#include <app_main.h>
#include <devices.h>
#include <main.h>
#include <stdio.h>


void cmain()
{
    log("Starting DotC Virtual Machine");
    devices::init(MEM_SIZE, DISK_SIZE);
}

void update()
{
    debug("Updating");
    devices::cpu->set<uint32_t>(devices::cpu->mem().at<uint32_t>(SCR_BUFFER + 720*480*4 - 5), INT32_MAX);
    devices::update();
}

void exit()
{
    log("Closing DotC Virtual Machine");

    // TODO: do this only if asked to
    log("Saving machine stat");
    FILE* statFile = fopen("dotcvm_stat.dat", "w+");
    fwrite(&(devices::cpu->mem()[0]), 1, devices::cpu->mem().size(), statFile);
    fclose(statFile);
    log("Saved machine stat");

    devices::exit();
    log("Bye");
}
