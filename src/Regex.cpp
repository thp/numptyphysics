#include "Regex.h"

#include "slre.h"

std::vector<std::string>
NP::Regex::match_groups(const std::string &re, const std::string &string, int ncaps, int flags)
{
    std::vector<std::string> result;
    slre_cap *caps = new slre_cap[ncaps];

    if (slre_match(re.c_str(), string.c_str(), string.length(), caps, ncaps, flags) > 0) {
        for (int i=0; i<ncaps; i++) {
            result.push_back(std::string(caps[i].ptr, caps[i].len));
        }
    }

    delete [] caps;
    return result;
}
