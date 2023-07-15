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

template<size_t index>
inline constexpr const char* file_name(const char* c, int)
{
    return (*c == '/') ? (c + 1) : file_name<index - 1>(c - 1, 0);
}

template<>
inline constexpr const char* file_name<0>(const char* c, int)
{
    return (*c == '/') ? (c + 1) : c;
}

template <size_t length>
inline constexpr const char* file_name(const char (&s)[length])
{
    return file_name<length - 2>(s + length - 2, 0);
}

#define LOG(lvl, message) \
    if (LOG_LVL <= lvl) \
    { \
        if (lvl == 1) \
            std::cout << "\033[1;34m[" << file_name(__FILE__) << ":" << __LINE__ << " - " << __func__ <<  "] " << message << "\033[0m" << std::endl; \
        if (lvl == 2) \
            std::cout << "\033[1;33m[" << file_name(__FILE__) << ":" << __LINE__ << " - " << __func__ <<  "] " << message << "\033[0m" << std::endl; \
        if (lvl == 3) \
            std::cout << "\033[1;31m[" << file_name(__FILE__) << ":" << __LINE__ << " - " << __func__ <<  "] " << message << "\033[0m" << std::endl; \
    } \

#endif