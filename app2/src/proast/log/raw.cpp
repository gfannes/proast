#include <proast/log/raw.hpp>
#include <optional>
#include <fstream>

namespace proast { namespace log { 
    std::ostream &ostream()
    {
        static std::optional<std::ofstream> fo;
        if (!fo)
            fo.emplace("proast.log");
        return *fo;
    }
} } 
