#ifndef LOG_H
#define LOG_H
#include <iostream>

#define log(x) std::cout << x << std::endl

#ifdef DEBUG
#define debug(x) std::cout << "DEBUG: " << x << '\n';
#else
#define debug(x)
#endif

#endif
