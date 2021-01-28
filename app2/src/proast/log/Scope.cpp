#include <proast/log/Scope.hpp>

namespace proast { namespace log { 
    unsigned int Scope::level__ = 0;

    Scope::Scope(const std::string &name, bool close_header)
    {
        ostream() << std::string(level__*2, ' ') << "[" << name << "]";
        if (close_header)
            ostream() << "{" << std::endl;
        ostream() << std::flush;
        ++level__;
    }

    Scope::~Scope()
    {
        --level__;
        ostream() << std::string(level__*2, ' ') << "}" << std::endl;
    }
} } 
