#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <string.h>
#include <app_main.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define log(x) std::cout << x << std::endl
#define err(x) std::cout << "Error at " << __FILENAME__ << ":" << __LINE__ << " " << x << '\n';requestClose()

#ifdef DEBUG
#define debug(x) std::cout << "DEBUG: " << x << '\n'
#else
#define debug(x)
#endif

#endif
