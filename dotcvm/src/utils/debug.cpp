#include <dotcvm/utils/log.hpp>
#include <dotcvm/dotcvm.hpp>
#include <csignal>

#ifdef DEBUG
void signal_handler(int sig_num)
{
    debug("Caught signal: " << sig_num);
    if(sig_num == SIGINT)
    {
        log("Program interrupted");
        exit(SIGINT);
        // TODO: shutdown
        return;
    }
    if(sig_num == SIGSEGV)
    {
        shutdown(SIGSEGV, "Memory error: Please report error");
        return;
    }
    debug("Unhandled signal, ignoring it");
}
#endif
void setup_signal_handler()
{
#ifdef DEBUG
    signal(SIGINT , signal_handler);
    signal(SIGSEGV, signal_handler);
#endif
}