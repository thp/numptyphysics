#include "thp_format.h"

#include <string>
#include <cstdarg>
#include <cstdlib>


std::string
thp::format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *tmp;
    vasprintf(&tmp, fmt, ap);
    std::string result = tmp;
    free(tmp);

    return result;
}

std::string
thp::format(const std::string &fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *tmp;
    vasprintf(&tmp, fmt.c_str(), ap);
    std::string result = tmp;
    free(tmp);

    return result;
}
