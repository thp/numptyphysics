#include "Interactions.h"

#include <sstream>
#include <cctype>

namespace NP {

Interactions::Interactions()
    : m_interactions()
{
}

Interactions::~Interactions()
{
}

bool
Interactions::handle(int color)
{
    auto it = m_interactions.find(color);
    if (it != m_interactions.end()) {
        printf("EVENT: '%s'\n", it->second.c_str());
        return true;
    }

    return false;
}

void
Interactions::clear()
{
}

bool
Interactions::parse(const std::string &line)
{
    // Interaction: 123 = main_menu
    //             ^^^^^^^^^^^^^^^^
    const char *tmp = line.c_str();
    while (*tmp && ::isspace(*tmp)) {
        tmp++;
    }
    if (*tmp < '0' || *tmp > '9') {
        return false;
    }

    int color = atoi(tmp);
    while (*tmp && *tmp != '=') {
        tmp++;
    }
    if (*tmp != '=') {
        return false;
    }
    tmp++;
    while (*tmp && ::isspace(*tmp)) {
        tmp++;
    }
    if (!*tmp) {
        return false;
    }
    std::string action(tmp);

    m_interactions[color] = action;
    return true;
}

std::string
Interactions::serialize()
{
    std::stringstream s;

    for (auto &interaction: m_interactions) {
        s << "Interaction: " << interaction.first << " = " << interaction.second << std::endl;
    }

    return s.str();
}

}; /* namespace NP */
