#include "I18n.h"

#include "Config.h"
#include "petals_log.h"
#include "thp_iterutils.h"

static bool
g_loaded = false;

static
std::map<std::string,std::string>
g_map;

static std::string
g_tmp;

const char *
Tr::c_str() const
{
    if (!key) {
        if (fmt) {
            g_tmp = fmt();
            return g_tmp.c_str();
        }

        return buf.c_str();
    }

    if (g_loaded) {
        auto it = g_map.find(std::string(key));
        if (it != g_map.end()) {
            return it->second.c_str();
        } else {
            LOG_WARNING("Untranslated: '%s'", key);
        }
    }

    return key;
}

Tr::operator bool() const
{
    return (key != nullptr) || buf.size();
}

Tr
Tr::copy(const std::string &s)
{
    Tr result(nullptr);
    result.buf = s;
    return result;
}

Tr
Tr::defer(std::function<std::string()> fmt)
{
    Tr result(nullptr);
    result.fmt = fmt;
    return result;
}

void
Tr::load(const std::string &filename)
{
    std::string data = Config::readFile(filename);

    g_map.clear();
    int lineno = 1, loaded = 0;
    for (auto &line: thp::split(thp::trim(data), "\n")) {
        std::string k, v;
        try {
            thp::unpack({&k, &v}) = thp::split(line, "=");
            g_map[k] = v;
            loaded++;
        } catch (const thp::UnpackException &e) {
            LOG_WARNING("Could not parse %s, line %d: '%s' - ignoring", filename.c_str(), lineno, line.c_str());
        }
        lineno++;
    }
    LOG_INFO("Loaded %d translations from %s", loaded, filename.c_str());
    g_loaded = true;
}
