#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <string.h>


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define log(x) std::cout << x << '\n'
#define err(x) std::cerr << "Error at " << __FILENAME__ << ":" << __LINE__ << ", function name: " << __func__ << ". Message: " << x << '\n'

#ifdef DEBUG
#define debug(x) std::cout << "DEBUG: " << x << '\n'
#else
#define debug(x)
#endif

#endif
