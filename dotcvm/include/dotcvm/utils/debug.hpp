#ifndef DC_DEBUG_H
#define DC_DEBUG_H

#include <csignal>
#include <iostream>

#ifdef DEBUG
#define BREAKPOINT(s) std::cout << "BREAKPOINT: " << s << '\n'; std::raise(SIGTRAP)
#else
#define BREAKPOINT(s)
#endif

#ifdef DOTCVM
void setup_signal_handler();
#endif /* DOTCVM */

#endif /* DC_DEBUG_H */