#ifndef THP_ITERUTILS_H
#define THP_ITERUTILS_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <functional>

namespace thp {

class UnpackException {
public:
    UnpackException(const std::string &message, int values, int targets)
        : message(message)
        , values(values)
        , targets(targets)
    {
    }

    std::string message;
    int values;
    int targets;
};

template <class T>
class Unpacker {
public:
    Unpacker(std::initializer_list<T *> l, std::vector<T> *rest=nullptr) : m_targets(l), m_rest(rest) {}

    Unpacker<T> &operator=(std::vector<T> values)
    {
        if ((m_rest == nullptr && values.size() != m_targets.size()) || values.size() < m_targets.size()) {
            throw UnpackException("Unpack fails", values.size(), m_targets.size());
        }

        for (int i=0; i<m_targets.size(); i++) {
            *(m_targets[i]) = values[i];
        }

        if (m_rest != nullptr) {
            for (int i=m_targets.size(); i < values.size(); i++) {
                m_rest->push_back(values[i]);
            }
        }

        return *this;
    }

private:
    std::vector<T *> m_targets;
    std::vector<T> *m_rest;
};

template <class T>
Unpacker<T> unpack(std::initializer_list<T *> l, std::vector<T> *rest=nullptr) {
    return Unpacker<T>(l, rest);
}

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

std::function<std::string(const std::string &)> trim = _trim;

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

template <class T, class C>
C map(std::function<T(const T &)> function, const C &sequence)
{
    C result;
    for (const T &v: sequence) {
        result.push_back(function(v));
    }
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

#endif /* THP_ITERUTILS_H */
