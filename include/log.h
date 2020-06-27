#ifndef LOG_H
#define LOG_H
#include <iostream>

#define log(x) std::cout << x << std::endl

#ifdef QT_QML_DEBUG
#define debug(x) std::cout << "DEBUG: " << x << '\n';
#endif
#ifndef QT_QML_DEBUG
#define debug(x)
#endif

#endif
