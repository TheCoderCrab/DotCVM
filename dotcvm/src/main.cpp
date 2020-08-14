#include <dotcvm/utils/log.hpp>
#include <dotcvm/utils/debug.hpp>
#include <dotcvm/core/modules.hpp>
#include <dotcvm/utils/string.hpp>

static bool s_shutdown = false;
static uint s_exit_code = 0;
static std::string s_exit_message = "";

void shutdown(uint exit_code, std::string message)
{
    s_shutdown = true;
    s_exit_code = exit_code;
    s_exit_message = message;
}

int main()
{    
    LOG_M("Starting dotcvm");
    setup_signal_handler();
    LOG_M("Loading modules");
    load_modules();
    LOG_M("Modules loaded");
    LOG_M("Loaded a total of: " << module_count() << " modules");
    if(module_count() == 0)
        shutdown(0, "Sir, please add some modules/devices, the dotcvm has no purpose if it is being used without any modules(try to turn on an empty computer case to see why)");
    // BREAKPOINT("Program loop will start");
    while(!s_shutdown)
        clock_modules();
    LOG_M("Shutting down");
    LOG_M("Unloading modules");
    BREAKPOINT("Exiting");
    unload_modules();
    LOG_M(s_exit_message);
    LOG_M("Bye");
    return s_exit_code;
}