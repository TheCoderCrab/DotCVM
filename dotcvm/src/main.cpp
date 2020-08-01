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
    log("Starting dotcvm");
    setup_signal_handler();
    log("Loading modules");
    load_modules();
    log("Modules loaded");
    while(!s_shutdown)
        clock_modules();
    log("Shutting down");
    log("Unloading modules");
    unload_modules();
    log(s_exit_message);
    log("Bye");
    return s_exit_code;
}