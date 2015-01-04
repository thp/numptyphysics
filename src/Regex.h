#ifndef NUMPTYPHYSICS_REGEX_H
#define NUMPTYPHYSICS_REGEX_H

#include <vector>
#include <string>

namespace NP {

namespace Regex {

std::vector<std::string>
match_groups(const std::string &re, const std::string &string, int ncaps, int flags=0);

} // namespace Regex

} // namespace NP

#endif /* NUMPTYPHYSICS_REGEX_H */
