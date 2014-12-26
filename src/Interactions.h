#ifndef NUMPTYPHYSICS_INTERACTIONS_H
#define NUMPTYPHYSICS_INTERACTIONS_H

#include <string>
#include <map>

namespace NP {

class Interactions {
public:
    Interactions();
    ~Interactions();

    bool handle(int color);
    void clear();

    bool parse(const std::string &line);
    bool add(const std::string &color, const std::string &action);
    std::string serialize();

private:
    std::map<int,std::string> m_interactions;
};

}; /* namespace NP */

#endif /* NUMPTYPHYSICS_INTERACTIONS_H */
