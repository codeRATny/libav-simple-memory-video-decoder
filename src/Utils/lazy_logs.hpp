#ifndef _LAZY_LOGS_H_
#define _LAZY_LOGS_H_
#include <iostream>

// levels:
// 0 - quiet
// 1 - info
// 2 - warning
// 3 - error
#define LOG_LVL 1

#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERR 3

#define LOG(prefix, lvl, message) \
    if (LOG_LVL <= lvl) \
    { \
        if (lvl == 1) \
            std::cout << "\033[1;34m" << prefix << ": " << message << "\033[0m" << std::endl; \
        if (lvl == 2) \
            std::cout << "\033[1;33m" << prefix << ": " << message << "\033[0m" << std::endl; \
        if (lvl == 3) \
            std::cout << "\033[1;31m" << prefix << ": " << message << "\033[0m" << std::endl; \
    } \

#endif