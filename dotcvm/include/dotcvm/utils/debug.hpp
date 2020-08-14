#ifndef DC_DEBUG_H
#define DC_DEBUG_H

#include <csignal>
#include <dotcvm/utils/log.hpp>

#ifdef DEBUG
#define BREAKPOINT(s) DEBUG_M(s); std::raise(SIGTRAP)
#else
#define BREAKPOINT(s)
#endif

#ifdef DOTCVM
void setup_signal_handler();
#endif /* DOTCVM */

#endif /* DC_DEBUG_H */