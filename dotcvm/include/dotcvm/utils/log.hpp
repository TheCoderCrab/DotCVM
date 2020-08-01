#ifndef DC_LOG_H
#define DC_LOG_H

#include <iostream>

#define log(s) std::cout << s << '\n'
#define warn(s) std::cout << "WARNING: " << s << '\n';
#define err(s) std::cerr << "ERROR: FILE: " << __FILE__ << ", LINE: " << __LINE__ << ", MESSAGE: " << s << '\n';

#ifdef DEBUG
#define debug(s) std::cout << "DEBUG: " << s << '\n'
#else
#define debug(s) do{}while(0)
#endif

#endif /* DC_LOG_H */