#ifndef HEADER_proast_model_Path_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Path_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>

namespace proast { namespace model { 

    using StringPath = std::vector<std::string>;

    std::string to_string(const StringPath &);
    StringPath to_string_path(const std::string &);

    bool pop_if(StringPath &path, const StringPath &needle);

} } 

#endif
