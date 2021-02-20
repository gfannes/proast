#ifndef HEADER_proast_types_hpp_ALREADY_INCLUDED
#define HEADER_proast_types_hpp_ALREADY_INCLUDED

#include <string>
#include <set>

namespace proast { 
    enum class Movement
    {
        Down, Up, Left, Right, Top, Bottom,
    };

    using Tag = std::string;
    using Tags = std::set<Tag>;
} 

#endif
