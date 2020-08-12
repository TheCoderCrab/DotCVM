#include <dotcvm/utils/log.hpp>
#include <dotcvm/main.hpp>
#include <dotcvm/core/modules.hpp>
#include <csignal>

void signal_handler(int sig_num)
{
    DEBUG_M("Caught signal: " << sig_num);
    if(sig_num == SIGINT)
    {
        shutdown(SIGINT, "Program Interrupted");
        return;
    }
    if(sig_num == SIGQUIT)
        DEBUG_M("Quitting");
    if(sig_num == SIGSEGV) 
    {
        WARN_M("Critical error caugth: SIGSEGV");
        WARN_M(get_module_error());
        exit(SIGSEGV);
        return;
    }
    WARN_M("Unhandled signal, ignoring it");
}

void setup_signal_handler()
{
    std::signal(SIGINT , signal_handler);
    std::signal(SIGQUIT, signal_handler);
#ifdef DEBUG
    std::signal(SIGSEGV, signal_handler);
#endif
}