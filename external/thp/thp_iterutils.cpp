/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2014 Thomas Perl <m@thp.io>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#if 0
exec clang++ -std=c++11 -DTHP_ITERUTILS_TEST -o thp_iterutils_test -x c++ $0
#endif

#include "thp_iterutils.h"

namespace thp {

std::string
_trim(const std::string &stdstr)
{
    const char *str = stdstr.c_str();
    const char *end = str + strlen(str) - 1;
    while (*str && isspace(*str)) {
        str++;
    }
    while (end > str && isspace(*end)) {
        end--;
    }
    return std::string(str, end + 1);
}

std::vector<std::string> split(const std::string &text, const std::string &sep) {
    std::vector<std::string> result;

    char *tmp = strdup(text.c_str());

    char *saveptr = NULL;
    char *c = strtok_r(tmp, sep.c_str(), &saveptr);
    while (c != NULL) {
        result.push_back(c);
        c = strtok_r(NULL, sep.c_str(), &saveptr);
    }

    free(tmp);

    return result;
}

}; // namespace thp


#if defined(THP_ITERUTILS_TEST)
int main(int argc, char *argv[])
{
    int a, b;
    thp::unpack({&a, &b}) = {3, 4};
    std::cerr << "Got: a = " << a << ", b = " << b << std::endl;

    std::string c, d;
    thp::unpack({&c, &d}) = thp::split("stroke:#ff00ff", ":");
    std::cerr << "Got: c = " << c << ", d = " << d << std::endl;

    try {
        std::string e, f;
        thp::unpack({&e, &f}) = {"A", "B", "C"};
    } catch (thp::UnpackException e) {
        std::cerr << "Cannot unpack, expected " << e.targets << ", got " << e.values << std::endl;
    }

    std::string g, h;
    std::vector<std::string> rest;
    thp::unpack({&g, &h}, &rest) = thp::split("a;b;c;d;e;f", ";");
    std::cerr << "g = " << g << ", h = " << h << ", rest size = " << rest.size() << std::endl;

    std::string k, v;
    thp::unpack({&k, &v}) = thp::map(thp::trim, thp::split(" stroke: #ff00ff ", ":"));
    std::cerr << "Got: '" << k << "', '" << v << "'" << std::endl;
}
#endif /* defined(THP_ITERUTILS_TEST) */
