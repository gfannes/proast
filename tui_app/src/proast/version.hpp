#ifndef HEADER_proast_version_hpp_ALREADY_INCLUDED
#define HEADER_proast_version_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { namespace version { 

    inline unsigned int major() { return 1; }
    inline unsigned int minor() { return 0; }
    inline unsigned int patch() { return 0; }

    std::string         git_hash();

} }

#endif
