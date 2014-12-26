#ifndef THP_FORMAT_H
#define THP_FORMAT_H

#include <functional>

namespace thp {

/* allocating sprintf */
std::string format(const char *fmt, ...);
std::string format(const std::string &fmt, ...);

}; /* namespace thp */

#endif /* THP_FORMAT_H */
