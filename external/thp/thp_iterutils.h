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

std::string _trim(const std::string &stdstr);
std::vector<std::string> split(const std::string &text, const std::string &sep);

static std::function<std::string(const std::string &)> trim = _trim;

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

#endif /* THP_ITERUTILS_H */
